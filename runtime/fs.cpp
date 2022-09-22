/*
 * Ham Runtime
 * Copyright (C) 2022  Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ham/fs.h"
#include "ham/memory.h"
#include "ham/log.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#	include <io.h>
#	define stat _stat64
#	define fstat _fstat64
#	define access _access
#	define open _open
#else
#	include <fcntl.h>
#	include <unistd.h>
#endif

#include <errno.h>
#include <magic.h>

#include <mutex>

using namespace ham::typedefs;
using namespace ham::literals;

HAM_C_API_BEGIN

static std::mutex ham_impl_magic_mutex;
static magic_t ham_impl_magic_cookie = nullptr;
static bool ham_impl_magic_init_flag = false;

static inline void ham_impl_magic_finish(){
	std::scoped_lock lock(ham_impl_magic_mutex);
	magic_close(ham_impl_magic_cookie);
	ham_impl_magic_cookie = nullptr;
	ham_impl_magic_init_flag = false;
}

static inline bool ham_impl_magic_init(){
	std::scoped_lock lock(ham_impl_magic_mutex);
	if(ham_impl_magic_init_flag) return true;

	if(!ham_impl_magic_cookie){
		ham_impl_magic_cookie = magic_open(MAGIC_MIME);
		if(ham_impl_magic_cookie == nullptr){
			ham_logerrorf("ham_file_info", "Error initializing libmagic: %s", strerror(errno));
			return false;
		}

		atexit(ham_impl_magic_finish);
	}

	if(magic_compile(ham_impl_magic_cookie, ham_null) != 0){
		ham_logwarnf("ham_file_info", "Error in magic_compile: %s", magic_error(ham_impl_magic_cookie));
		if(magic_load(ham_impl_magic_cookie, ham_null) != 0){
			ham_logerrorf("ham_file_info", "Error in magic_load: %s", magic_error(ham_impl_magic_cookie));
			return false;
		}
	}

	ham_impl_magic_init_flag = true;
	return true;
}

struct ham_file{
	const ham_allocator *allocator;
	int fd;
};

bool ham_path_exists_utf8(ham_str8 path){
	ham_path_buffer_utf8 path_buf;
	if(path.len >= HAM_PATH_BUFFER_SIZE) return false;

	memcpy(path_buf, path.ptr, path.len);
	path_buf[path.len] = '\0';

	return access(path_buf, F_OK) == 0;
}

bool ham_path_exists_utf16(ham_str16 path){
	ham_path_buffer_utf8 path_buf;

	const usize path_len = ham_str_conv_utf16_utf8(path, path_buf, sizeof(path_buf)-1);
	if(path_len == (usize)-1) return false;

	return access(path_buf, F_OK) == 0;
}

bool ham_path_exists_utf32(ham_str32 path){
	ham_path_buffer_utf8 path_buf;

	const usize path_len = ham_str_conv_utf32_utf8(path, path_buf, sizeof(path_buf)-1);
	if(path_len == (usize)-1) return false;

	return access(path_buf, F_OK) == 0;
}

static inline bool ham_impl_fstat(int fd, ham_file_info *ret){
	struct stat stat_buf;
	const int res = fstat(fd, &stat_buf);
	if(res != 0){
		ham_logapierrorf("Error in fstat: %s\n", strerror(errno));
		return false;
	}

	switch(stat_buf.st_mode & S_IFMT){
		case S_IFDIR:{
			ret->kind = HAM_FILE_DIRECTORY;
			break;
		}

		case S_IFREG:{
			ret->kind = HAM_FILE_REGULAR;
			break;
		}

		case S_IFLNK:{
			ret->kind = HAM_FILE_LINK;
			break;
		}

		default:{
			ret->kind = HAM_FILE_UNKNOWN;
			break;
		}
	}

	ret->size = (usize)stat_buf.st_size;

	if(ham_impl_magic_init()){
		const char *mime_str = magic_descriptor(ham_impl_magic_cookie, fd);
		if(!mime_str){
			ham_logapiwarnf("Error in magic_descriptor: %s", magic_error(ham_impl_magic_cookie));
			ret->mime = HAM_EMPTY_STR8;
		}
		else{
			ret->mime = ham::str8(mime_str);
		}
	}
	else{
		ham_logapiwarnf("Failed to initialize libmagic, no mime info returned.");
		ret->mime = HAM_EMPTY_STR8;
	}

	return true;
}

static inline bool ham_impl_stat(const char *path, ham_file_info *ret){
	const int fd = open(path, O_RDONLY);
	if(fd == -1){
		ham_logapierrorf("Error in open: %s\n", strerror(errno));
		return false;
	}

	if(!ham_impl_fstat(fd, ret)){
		return false;
	}

	const int res = close(fd);
	if(res != 0){
		ham_logapiwarnf("Error in close: %s\n", strerror(errno));
	}

	return true;
}

bool ham_path_file_info_utf8 (ham_str8  path, ham_file_info *ret){
	if(!path.ptr || !path.len || !ret) return false;

	ham_path_buffer_utf8 path_buf;
	if(path.len >= HAM_PATH_BUFFER_SIZE) return false;

	memcpy(path_buf, path.ptr, path.len);
	path_buf[path.len] = '\0';

	if(!ham_impl_stat(path_buf, ret)){
		ham_logapierrorf("Internal error in ham_impl_stat");
		return false;
	}

	return true;
}

bool ham_path_file_info_utf16(ham_str16 path, ham_file_info *ret){
	if(!path.ptr || !path.len || !ret) return false;

	ham_path_buffer_utf8 path_buf;

	const usize path_len = ham_str_conv_utf16_utf8(path, path_buf, sizeof(path_buf)-1);
	if(path_len == (usize)-1) return false;

	if(!ham_impl_stat(path_buf, ret)){
		ham_logapierrorf("Internal error in ham_impl_stat");
		return false;
	}

	return true;
}

bool ham_path_file_info_utf32(ham_str32 path, ham_file_info *ret){
	if(!path.ptr || !path.len || !ret) return false;

	ham_path_buffer_utf8 path_buf;

	const usize path_len = ham_str_conv_utf32_utf8(path, path_buf, sizeof(path_buf)-1);
	if(path_len == (usize)-1) return false;

	if(!ham_impl_stat(path_buf, ret)){
		ham_logapierrorf("Internal error in ham_impl_stat");
		return false;
	}

	return true;
}

ham_file *ham_file_open_utf8 (ham_str8  path, ham_u32 flags){
	if(!path.ptr || !path.len) return nullptr;

	ham_path_buffer_utf8 path_buf;
	if(path.len >= HAM_PATH_BUFFER_SIZE) return nullptr;

	memcpy(path_buf, path.ptr, path.len);
	path_buf[path.len] = '\0';

	int open_flags = 0;

	if((flags & HAM_OPEN_RDWR) == HAM_OPEN_RDWR){
		open_flags |= O_RDWR;
	}
	else if(flags & HAM_OPEN_READ){
		open_flags |= O_RDONLY;
	}
	else if(flags & HAM_OPEN_WRITE){
		open_flags |= O_WRONLY;
	}
	else{
		// TODO: signal warning
		open_flags |= O_RDWR;
	}

	int fd = open(path_buf, open_flags);
	if(fd == -1){
		// TODO: signal error
		return nullptr;
	}

	const ham::allocator<ham_file> allocator;

	const auto mem = allocator.allocate();
	if(!mem){
		if(close(fd) != 0){
			// TODO: signal warning
		}
		return nullptr;
	}

	const auto ptr = allocator.construct(mem);
	if(!ptr){
		if(close(fd) != 0){
			// TODO: signal warning
		}
		allocator.deallocate(mem);
		return nullptr;
	}

	ptr->allocator = allocator;
	ptr->fd = fd;

	return ptr;
}

//ham_file *ham_file_open_utf16(ham_str16 path, ham_u32 flags);
//ham_file *ham_file_open_utf32(ham_str32 path, ham_u32 flags);

void ham_file_close(ham_file *file){
	if(!file) return;

	if(close(file->fd) != 0){
		// TODO: signal warning
	}

	const ham::allocator<ham_file> allocator = file->allocator;

	allocator.destroy(file);
	allocator.deallocate(file);
}

bool ham_file_get_info(const ham_file *file, ham_file_info *ret){
	if(!file || !ret) return false;

	if(!ham_impl_fstat(file->fd, ret)){
		ham_logapierrorf("Internal error in ham_impl_stat");
		return false;
	}

	return true;
}

ham_usize ham_file_read(ham_file *file, void *buf, ham_usize max_len){
	if(!file) return (usize)-1;
	else if(!max_len) return 0;
	else if(!buf) return (usize)-1;

	const ssize_t res = read(file->fd, buf, max_len);
	if(res == -1){
		// TODO: signal error
		return (usize)-1;
	}

	return (usize)res;
}

ham_usize ham_file_write(ham_file *file, const void *buf, ham_usize len){
	if(!file) return (usize)-1;
	else if(!len) return 0;
	else if(!buf) return (usize)-1;

	const ssize_t res = write(file->fd, buf, len);
	if(res == -1){
		// TODO: signal error
		return (usize)-1;
	}

	return (usize)res;
}

ham_usize ham_file_seek(ham_file *file, ham_usize off){
	if(!file) return (usize)-1;

	const off_t res = lseek(file->fd, off, SEEK_SET);
	if(res == (off_t)-1){
		// TODO: signal error
		return (usize)-1;
	}

	return (usize)res;
}

ham_usize ham_file_tell(const ham_file *file){
	if(!file) return (usize)-1;

	const off_t res = lseek(file->fd, 0, SEEK_CUR);
	if(res == (off_t)-1){
		// TODO: signal error
		return (usize)-1;
	}

	return (usize)res;
}

HAM_C_API_END
