#include "ham/object.h"
#include "ham/check.h"

#include "ham/std_vector.hpp"

#include <stdexcept>
#include <mutex>
#include <algorithm>

HAM_C_API_BEGIN

struct ham_object_manager{
	ham_object_manager(const ham_allocator *allocator, const ham_object_vtable *obj_vtable_)
		: obj_vtable(obj_vtable_)
		, obj_info(obj_vtable_->info())
		, instances(allocator)
		, allocated_blocks(0)
	{
		if(ham_popcnt64(obj_info->alignment) != 1){
			ham_logerrorf("ham_object_manager_create", "Invalid object alignment: %zu", obj_info->alignment);
			throw std::bad_alloc();
		}
		else if(obj_info->size % obj_info->alignment != 0){
			ham_logerrorf(
				"ham_object_manager_create",
				"Unaligned object size: %zu (alignment remainder %zu)",
				obj_info->size, obj_info->size % obj_info->alignment
			);
			throw std::bad_alloc();
		}

		const auto page_size = ham_get_page_size();
		if(obj_info->size > page_size){
			ham_logerrorf("ham_object_manager_create", "Object too large: %zu, max (page size) %zu", obj_info->size, page_size);
			throw std::bad_alloc();
		}

		blocks[0] = ham_map_pages(1);

		if(!blocks[0]){
			ham_logerrorf("ham_object_manager_create", "Failed to allocate memory block");
			throw std::bad_alloc();
		}

		++allocated_blocks;
	}

	ham_object_manager(const ham_object_manager&) = delete;

	ham_object_manager(ham_object_manager &&other) noexcept
		: obj_vtable(other.obj_vtable)
		, obj_info(other.obj_info)
	{
		std::scoped_lock lock(other.mut);

		instances = std::move(other.instances);

		allocated_blocks = other.allocated_blocks;
		memcpy(blocks, other.blocks, sizeof(blocks));

		other.allocated_blocks = 0;
		memset(other.blocks, 0, sizeof(other.blocks));
	}

	~ham_object_manager(){
		for(auto inst : instances){
			obj_vtable->destroy(inst);
		}

		for(ham_usize i = 0; i < allocated_blocks; i++){
			const auto block_num_pages = 1UL << i;
			const auto block = blocks[i];
			ham_unmap_pages(block, block_num_pages);
		}
	}

	ham_object_manager &operator=(ham_object_manager &&other) noexcept{
		if(this != &other){
			std::scoped_lock lock(mut, other.mut);

			clear_unsafe();

			obj_vtable = other.obj_vtable;
			obj_info = other.obj_info;

			other.obj_vtable = nullptr;
			other.obj_info = nullptr;

			instances = std::move(other.instances);

			allocated_blocks = other.allocated_blocks;
			memcpy(blocks, other.blocks, sizeof(blocks));

			other.allocated_blocks = 0;
			memset(other.blocks, 0, sizeof(other.blocks));
		}

		return *this;
	}

	void clear_unsafe() noexcept{
		for(auto inst : instances){
			obj_vtable->destroy(inst);
		}

		for(ham_usize i = 0; i < allocated_blocks; i++){
			const auto block_num_pages = 1UL << i;
			const auto block = blocks[i];
			ham_unmap_pages(block, block_num_pages);
		}

		allocated_blocks = 0;
	}

	ham_usize find_block_unsafe(const void *ptr) const noexcept{
		if(!ptr) return (ham_usize)-1;

		const auto page_size = ham_get_page_size();
		const auto ptr_val = (ham_uptr)ptr;

		for(ham_usize i = 0; i < allocated_blocks; i++){
			const auto block_size = page_size * (1UL << i);
			const auto block_val = (ham_uptr)blocks[i];
			if(ptr_val < block_val || ptr_val >= (block_val + block_size)){
				continue;
			}

			return i;
		}

		return (ham_usize)-1;
	}

	ham_usize allocate_block_unsafe(void **ret) noexcept{
		const auto new_idx = allocated_blocks;
		if(new_idx >= HAM_OBJECT_INSTANCE_MANAGER_MAX_BLOCKS){
			return (ham_usize)-1;
		}

		void *const new_block = ham_map_pages(1UL << new_idx);
		if(!new_block){
			return (ham_usize)-1;
		}

		*ret = new_block;

		blocks[new_idx] = new_block;
		++allocated_blocks;

		return new_idx;
	}

	bool fits_block(ham_usize block_idx, ham_usize n, const void *ptr) const noexcept{
		if(block_idx >= HAM_OBJECT_INSTANCE_MANAGER_MAX_BLOCKS) return false;

		const auto beg_uval = (ham_uptr)ptr;
		const auto end_uval = beg_uval + (obj_info->size * n);

		const auto block_size = ham_get_page_size() * (1UL << block_idx);
		const auto block_uval = (ham_uptr)blocks[block_idx];

		if(beg_uval < block_uval || end_uval > (block_uval + block_size)){
			return false;
		}

		return true;
	}

	ham_object *create(ham_usize nargs, va_list va){
		std::scoped_lock lock(mut);

		void *new_ptr;

		if(instances.empty()){
			new_ptr = blocks[0];
		}
		else{
			const auto inst_end = instances.end();

			for(auto inst_it = instances.begin(); inst_it != inst_end; ++inst_it){
				const auto next_inst = inst_it + 1;

				void *const end_ptr = (char*)*inst_it + obj_info->size;

				if(next_inst < inst_end && (ham_uptr)*next_inst <= (ham_uptr)end_ptr){
					continue;
				}

				new_ptr = end_ptr;
				break;
			}

			ham_usize block_idx = find_block_unsafe(new_ptr);
			if(block_idx == (ham_usize)-1){
				block_idx = allocate_block_unsafe(&new_ptr);
				if(block_idx == (ham_usize)-1){
					ham_logerrorf("ham_object_create", "Out of memory");
					return nullptr;
				}
			}
		}

		const auto ret = obj_vtable->construct((ham_object*)new_ptr, nargs, va);

		if(!ret){
			ham_logerrorf("ham_object_create", "Error constructing object");
			return nullptr;
		}

		const auto insert_it = std::upper_bound(instances.begin(), instances.end(), (ham_object*)new_ptr);
		instances.insert(insert_it, (ham_object*)new_ptr);

		return ret;
	}

	bool destroy(ham_object *obj) noexcept{
		std::scoped_lock lock(mut);

		const auto inst_it = std::lower_bound(instances.begin(), instances.end(), obj);
		if(inst_it == instances.end() || *inst_it != obj) return false;

		const auto block_idx = find_block_unsafe(obj);
		if(block_idx == (ham_usize)-1) return false;

		obj_vtable->destroy(obj);
		instances.erase(inst_it);

		// TODO: free empty blocks?

		//void *const block_end_ptr = (char*)blocks[block_idx] + (1UL << block_idx);

		return true;
	}

	const ham_object_vtable *obj_vtable;
	const ham_object_info *obj_info;
	ham::std_vector<ham_object*> instances;

	ham_usize allocated_blocks;
	void *blocks[16];

	mutable std::recursive_mutex mut;
};

ham_object *ham_object_create(ham_object_manager *manager, ham_usize nargs, ...){
	if(!ham_check(manager != NULL)) return nullptr;

	va_list va;
	va_start(va, nargs);
	const auto ret = manager->create(nargs, va);
	va_end(va);

	return ret;
}

ham_object_manager *ham_object_manager_create(const ham_object_vtable *vtable){
	if(!ham_check(vtable != NULL)) return nullptr;

	const auto allocator = ham::allocator<ham_object_manager>();

	const auto mem = allocator.allocate(1);
	if(!mem){
		ham_logapierrorf("Out of memory");
		return nullptr;
	}

	const auto ptr = allocator.construct(mem, allocator, vtable);
	if(!ptr){
		ham_logapierrorf("Error constructing ham_object_manager");
		return nullptr;
	}

	return ptr;
}

void ham_object_manager_destroy(ham_object_manager *manager){
	if(!manager) return;

	const auto allocator = ham::allocator<ham_object_manager>(manager->instances.get_allocator());

	allocator.destroy(manager);
	allocator.deallocate(manager);
}

bool ham_object_manager_contains(const ham_object_manager *manager, const ham_object *obj){
	if(!ham_check(manager != NULL) || !ham_check(obj != NULL)){
		return false;
	}

	std::scoped_lock lock(manager->mut);
	return manager->find_block_unsafe(obj) != (ham_usize)-1;
}

ham_usize ham_object_manager_block_index(const ham_object_manager *manager, const ham_object *obj){
	if(!ham_check(manager != NULL) || !ham_check(obj != NULL)){
		return (ham_usize)-1;
	}

	std::scoped_lock lock(manager->mut);
	return manager->find_block_unsafe(obj);
}

ham_object *ham_object_vnew(ham_object_manager *manager, ham_usize nargs, va_list va){
	if(!ham_check(manager != NULL)){
		return nullptr;
	}

	return manager->create(nargs, va);
}

bool ham_object_delete(ham_object_manager *manager, ham_object *obj){
	if(!ham_check(manager != NULL) || !ham_check(obj != NULL)){
		return false;
	}

	return manager->destroy(obj);
}

HAM_C_API_END
