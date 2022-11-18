/*
 * Ham Runtime
 * Copyright (C) 2022 Keith Hammond
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

#include "ham/image.h"
#include "ham/check.h"
#include "ham/memory.h"
#include "ham/fs.h"
#include "ham/hash.h"

#include <mutex>

#include "FreeImage.h"

using namespace ham::typedefs;
using namespace ham::hash_literals;

static std::once_flag m_impl_freeimage_flag;

static inline void ham_freeimage_debug_handler(FREE_IMAGE_FORMAT fif, const char *message){
	const auto api = ham::format("FreeImage[{}]", FreeImage_GetFormatFromFIF(fif));
	ham_logerrorf(api.c_str(), "%s", message);
}

static inline void ham_freeimage_ensure(){
	std::call_once(m_impl_freeimage_flag, []{
		FreeImage_Initialise();
		std::atexit(FreeImage_DeInitialise);

		FreeImage_SetOutputMessage(ham_freeimage_debug_handler);
	});
}

HAM_C_API_BEGIN

union ham_image_data{
	struct {
		ham_color_format format;
		ham_u32 w, h;
		void *pixels;
	} stored;

	const ham_image *view;
};

struct ham_image{
	const ham_allocator *allocator;
	bool is_stored;
	ham_image_data data;
};

static inline FREE_IMAGE_FORMAT ham_fif_from_mime_type(const ham::str8 &mime){
	if(mime.is_empty()) return FIF_UNKNOWN;

	const auto semicolon_pos = mime.find(";");
	if(semicolon_pos != ham::str8::npos){
		return ham_fif_from_mime_type(mime.substr(0, semicolon_pos));
	}

	switch(ham::hash(mime)){
		case ham::hash(HAM_MIME_TYPE_BMP):  return FIF_BMP;
		case ham::hash(HAM_MIME_TYPE_ICO):  return FIF_ICO;
		case ham::hash(HAM_MIME_TYPE_JPEG): return FIF_JPEG;
		case ham::hash(HAM_MIME_TYPE_PNG):  return FIF_PNG;
		case ham::hash(HAM_MIME_TYPE_PPM):  return FIF_PPM;
		case ham::hash(HAM_MIME_TYPE_TIFF): return FIF_TIFF;
		case ham::hash(HAM_MIME_TYPE_WEBP): return FIF_WEBP;
		default: return FIF_UNKNOWN;
	}
}

static inline int ham_fif_load_flags(FREE_IMAGE_FORMAT fif){
	switch(fif){
	#ifdef HAM_DEBUG
		case FIF_JPEG: return JPEG_FAST | JPEG_EXIFROTATE;
	#else
		case FIF_JPEG: return JPEG_ACCURATE | JPEG_EXIFROTATE;
	#endif
		default: return 0;
	}
}

ham_image *ham_image_create_alloc(const ham_allocator *allocator, ham_color_format format, ham_u32 w, ham_u32 h, const void *data){
	if(!ham_check(format < HAM_COLOR_FORMAT_COUNT) || !ham_check(w != 0) || !ham_check(h != 0)){
		return nullptr;
	}

	const auto num_components = ham_color_format_num_components(format);
	const auto component_size = ham_color_format_component_size(format);
	const auto texture_size = w * h * num_components * component_size;

	const auto pixels = ham_allocator_alloc(allocator, alignof(void*), texture_size);
	if(!pixels){
		ham_logapierrorf("Failed to allocate pixel memory");
		return nullptr;
	}

	if(data){
		memcpy(pixels, data, texture_size);
	}
	else{
		memset(pixels, 0, texture_size);
	}

	const auto img = ham_allocator_new(allocator, ham_image);

	img->allocator = allocator;
	img->is_stored = true;

	img->data = ham_image_data{
		.stored = {
			.format = format,
			.w = w,
			.h = h,
			.pixels = pixels,
		}
	};

	return img;
}

ham_image *ham_image_create_view(const ham_image *img){
	const auto allocator = ham_current_allocator();

	const auto ret = ham_allocator_new(allocator, ham_image);
	if(!ret) return nullptr;

	ret->allocator = allocator;
	ret->is_stored = false;
	ret->data.view = img;

	return ret;
}

ham_image *ham_image_load_from_mem(ham_color_format format, ham_usize len, const void *data){
	if(!ham_check(len > 0) || !ham_check(data != NULL)){
		return nullptr;
	}

	if(format != HAM_RGBA8U){
		ham_logapierrorf("Only RGBA8 color format currently supported");
		return nullptr;
	}

	ham_freeimage_ensure();

	const ham::str8 mime_str = ham_mime_from_mem(len, data);

	const auto fif = ham_fif_from_mime_type(mime_str);
	if(fif == FIF_UNKNOWN){
		ham_logapierrorf("Unknown image format");
		return nullptr;
	}

	const auto fimem = FreeImage_OpenMemory(reinterpret_cast<BYTE*>(const_cast<void*>(data)), len);
	if(!fimem){
		ham_logapierrorf("Error in FreeImage_OpenMemory");
		return nullptr;
	}

	auto fib = FreeImage_LoadFromMemory(fif, fimem, ham_fif_load_flags(fif));
	if(!fib){
		ham_logapierrorf("Error in FreeImage_LoadFromMemory");
		FreeImage_CloseMemory(fimem);
		return nullptr;
	}

	// SCREW THIS.
	// for now convert to RGBA8
	// TODO: convert to 'format'

	const auto color_type = FreeImage_GetColorType(fib);
	const auto n_components = ham_color_format_num_components(format);
	const auto comp_size = ham_color_format_component_size(format);

	bool good_type = true;

	switch(color_type){
		case FIC_RGB:{
			if(n_components < 3){
				ham_logapierrorf("Could not convert RGB image to %d component image", n_components);
				good_type = false;
			}

			break;
		}

		case FIC_RGBALPHA:{
			if(n_components != 4){
				ham_logapierrorf("Could not convert RGBA image to %d component image", n_components);
				good_type = false;
			}

			break;
		}

		default:{
			good_type = false;
			ham_logapierrorf("Unrecognized image color type 0x%x", color_type);
			break;
		}
	}

	if(!good_type){
		FreeImage_Unload(fib);
		FreeImage_CloseMemory(fimem);
		return nullptr;
	}

	switch(format){
		case HAM_RGBA8U:
		case HAM_RGBA8I:{
			const auto old_fib = fib;
			fib = FreeImage_ConvertTo32Bits(fib);
			if(fib != old_fib){
				FreeImage_Unload(old_fib);
			}

			break;
		}

		case HAM_RGB8U:
		case HAM_RGB8I:{
			const auto old_fib = fib;
			fib = FreeImage_ConvertTo24Bits(fib);
			if(fib != old_fib){
				FreeImage_Unload(old_fib);
			}

			break;
		}

		case HAM_RGB16U:
		case HAM_RGB16I:{
			const auto old_fib = fib;
			fib = FreeImage_ConvertToRGB16(fib);
			if(fib != old_fib){
				FreeImage_Unload(old_fib);
			}

			break;
		}

		case HAM_RGBA16U:
		case HAM_RGBA16I:{
			const auto old_fib = fib;
			fib = FreeImage_ConvertToRGBA16(fib);
			if(fib != old_fib){
				FreeImage_Unload(old_fib);
			}

			break;
		}

		case HAM_RGB32F:
		case HAM_RGBA32F:{
			const auto old_fib = fib;
			fib = FreeImage_ConvertToFloat(fib);
			if(fib != old_fib){
				FreeImage_Unload(old_fib);
			}
		}

		default:{
			ham_logapierrorf("Only RGB8 and RGBA8 formats currently supported");
			FreeImage_Unload(fib);
			FreeImage_CloseMemory(fimem);
			return nullptr;
		}
	}

	BYTE *const fib_pixels = FreeImage_GetBits(fib);

	const u32 w = FreeImage_GetWidth(fib);
	const u32 h = FreeImage_GetHeight(fib);

	const auto allocator = ham_current_allocator();

	const auto pixels = ham_allocator_alloc(allocator, alignof(void*), w * h * n_components * comp_size);

	const u32 pitch = FreeImage_GetPitch(fib);

	for(u32 y = 0; y < h; y++){
		for(u32 x = 0; x < w; x++){
			const auto idx = ((y * w) + x) * n_components * comp_size;
			const auto fib_idx = (y * pitch) + (x * n_components * comp_size);
			// TODO: copy lines at a time at least :/
			memcpy((BYTE*)pixels + idx, fib_pixels + fib_idx, n_components * comp_size); // copy single pixel
		}
	}

	const auto img = ham_allocator_new(allocator, ham_image);

	img->allocator = allocator;
	img->is_stored = true;

	img->data = ham_image_data{
		.stored = {
			.format = format,
			.w = w,
			.h = h,
			.pixels = pixels,
		}
	};

	FreeImage_Unload(fib);
	FreeImage_CloseMemory(fimem);

	return img;
}

ham_image *ham_image_load(ham_color_format format, ham_str8 filepath){
	if(!ham_check(filepath.len > 0) || !ham_check(filepath.ptr != NULL)){
		return nullptr;
	}

	ham_freeimage_ensure();

	auto file = ham::file(filepath, ham::file_open_flags::read);

	ham_file_info file_info;
	if(!ham_file_get_info(file.ptr(), &file_info)){
		ham_logapierrorf("Error in ham_file_get_info");
		return nullptr;
	}

	const auto mapping = ham_file_map(file.ptr(), HAM_OPEN_READ, 0, file_info.size);

	const auto ret = ham_image_load_from_mem(format, file_info.size, mapping);

	if(!ham_file_unmap(file.ptr(), mapping, file_info.size)){
		ham_logapiwarnf("Error in ham_file_unmap");
	}

	return ret;
}

void ham_image_destroy(ham_image *img){
	if(ham_unlikely(!img)) return;

	const auto allocator = img->allocator;

	if(img->is_stored){
		ham_allocator_free(allocator, img->data.stored.pixels);
	}

	ham_allocator_delete(allocator, img);
}

const void *ham_image_pixels(const ham_image *img){
	if(!ham_check(img != NULL)) return nullptr;
	return img->is_stored ? img->data.stored.pixels : ham_image_pixels(img->data.view);
}

ham_u32 ham_image_width(const ham_image *img){
	if(!ham_check(img != NULL)) return (ham_u32)-1;
	return img->is_stored ? img->data.stored.w : ham_image_width(img->data.view);
}

ham_u32 ham_image_height(const ham_image *img){
	if(!ham_check(img != NULL)) return (ham_u32)-1;
	return img->is_stored ? img->data.stored.h : ham_image_height(img->data.view);
}

HAM_C_API_END
