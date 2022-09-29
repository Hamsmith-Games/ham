#include "ham/colony.h"
#include "ham/check.h"
#include "ham/memory.h"
#include "ham/async.h"

#include "ham/std_vector.hpp"

HAM_C_API_BEGIN

static inline ham_usize ham_impl_colony_get_bucket_size(ham_usize idx) noexcept{
	static const ham_usize page_size = ham_get_page_size();
	return page_size * (1UL << idx);
}

struct ham_colony{
	const ham_allocator *allocator;
	ham_usize obj_alignment, obj_size;

	ham_usize num_buckets;
	void *buckets[HAM_COLONY_MAX_BUCKETS];

	ham::mutex mut;
	ham::std_vector<std::pair<ham_usize, void*>> elements;
};

ham_nonnull_args(1, 2)
static inline ham_usize ham_impl_colony_find_bucket(const ham_colony *colony, const void *ptr){
	if((ham_uptr)ptr % colony->obj_alignment != 0) return (ham_usize)-1;

	const auto ptr_beg = (const char*)ptr;
	const auto ptr_end = (const char*)ptr + colony->obj_size;

	for(ham_usize i = 0; i < colony->num_buckets; i++){
		const auto bucket_size = ham_impl_colony_get_bucket_size(i);
		const auto bucket     = (const char*)colony->buckets[i];
		const auto bucket_end = (const char*)bucket + bucket_size;
		if(ptr_beg >= bucket && ptr_end < bucket_end){
			return (ham_usize)i;
		}
	}

	return (ham_usize)-1;
}

ham_nonnull_args(1)
static inline ham_usize ham_impl_colony_new_bucket(ham_colony *colony){
	if(colony->num_buckets == HAM_COLONY_MAX_BUCKETS) return (ham_usize)-1;

	const auto new_idx = colony->num_buckets;
	const auto num_pages = 1UL << new_idx;
	const auto new_bucket = ham_map_pages(num_pages);
	if(!new_bucket) return (ham_usize)-1;

	colony->buckets[new_idx] = new_bucket;
	++colony->num_buckets;
	return new_idx;
}

ham_colony *ham_colony_create(ham_usize obj_alignment, ham_usize obj_size){
	if(
	   !ham_check(ham_popcnt64(obj_alignment) == 1) ||
	   !ham_check(obj_size % obj_alignment == 0) ||
	   !ham_check(obj_size < ham_get_page_size())
	){
		return nullptr;
	}

	const auto allocator = ham_current_allocator();

	const auto ptr = ham_allocator_new(allocator, ham_colony);
	if(!ptr){
		return nullptr;
	}

	const auto bucket0 = ham_map_pages(1);
	if(!bucket0){
		ham_logapierrorf("Error allocating storage bucket");
		ham_allocator_delete(allocator, ptr);
		return nullptr;
	}

	ptr->allocator = allocator;
	ptr->obj_alignment = obj_alignment;
	ptr->obj_size = obj_size;
	ptr->num_buckets = 1;
	memset(ptr->buckets + 1, 0, sizeof(ptr->buckets) - sizeof(void*));
	ptr->buckets[0] = bucket0;
	ptr->elements = ham::std_vector<std::pair<ham_usize, void*>>(allocator);

	return ptr;
}

ham_nothrow void ham_colony_destroy(ham_colony *colony){
	if(ham_unlikely(colony == NULL)) return;

	const auto allocator = colony->allocator;

	for(int i = 0; i < colony->num_buckets; i++){
		const auto bucket = colony->buckets[i];
		const auto num_pages = (ham_usize)1 << (ham_usize)i;
		ham_unmap_pages(bucket, num_pages);
	}

	ham_allocator_delete(allocator, colony);
}

void *ham_colony_emplace(ham_colony *colony){
	if(!ham_check(colony != NULL)) return nullptr;

	ham::scoped_lock lock(colony->mut);

	if(colony->elements.empty()){
		const auto elem = colony->buckets[0];
		colony->elements.emplace_back(0, elem);
		return elem;
	}

	for(ham_usize i = 0; i < colony->elements.size(); i++){
		const auto &elem = colony->elements[i];
		const auto elem_bucket = colony->buckets[elem.first];
		const auto elem_bucket_end = (char*)elem_bucket + ham_impl_colony_get_bucket_size(elem.first);

		const auto req_ptr = (char*)elem.second + colony->obj_size;
		const auto req_ptr_end = req_ptr + colony->obj_size;
		if(req_ptr_end >= elem_bucket_end) continue;

		if((i + 1) != colony->elements.size()){
			const auto &next_elem = colony->elements[i + 1];
			if(next_elem.second == req_ptr){
				continue;
			}
		}

		// this is a sorted insert
		const auto res = colony->elements.insert(colony->elements.begin() + (i + 1), std::make_pair(elem.first, req_ptr));
		if(res == colony->elements.end()){
			ham_logapierrorf("Error inserting new allocation");
			return nullptr;
		}

		return req_ptr;
	}

	const auto new_bucket_idx = ham_impl_colony_new_bucket(colony);
	if(new_bucket_idx == (ham_usize)-1){
		ham_logapierrorf("Error allocating new bucket");
		return nullptr;
	}

	const auto new_elem_ptr = colony->buckets[new_bucket_idx];

	// this is a sorted insert too
	colony->elements.emplace_back(std::make_pair(new_bucket_idx, new_elem_ptr));

	return new_elem_ptr;
}

ham_nothrow bool ham_colony_erase(ham_colony *colony, void *ptr){
	if(!ham_check(colony != NULL)) return false;
	else if(!ptr) return false;

	ham::scoped_lock lock(colony->mut);

	const auto bucket_idx = ham_impl_colony_find_bucket(colony, ptr);
	if(bucket_idx == (ham_usize)-1){
		return false;
	}

	const auto req_elem = std::make_pair(bucket_idx, ptr);

	const auto elem_res = std::lower_bound(colony->elements.begin(), colony->elements.end(), req_elem);
	if(elem_res == colony->elements.end() || elem_res->second != ptr){
		return false;
	}

	colony->elements.erase(elem_res);
	return true;
}

ham_nothrow bool ham_colony_contains(const ham_colony *colony, const void *ptr){
	if(!ham_check(colony != NULL)) return false;
	else if(!ptr) return false;

	const auto bucket_idx = ham_impl_colony_find_bucket(colony, ptr);
	if(bucket_idx == (ham_usize)-1) return false;

	const auto req_elem = std::make_pair(bucket_idx, const_cast<void*>(ptr));

	const auto elem_res = std::lower_bound(colony->elements.begin(), colony->elements.end(), req_elem);
	if(elem_res == colony->elements.end() || (const void*)elem_res->second != ptr){
		return false;
	}

	return true;
}

bool ham_colony_view_erase(ham_colony *colony, void *ptr, ham_colony_iterate_fn view_fn, void *user){
	if(!ham_check(colony != NULL)) return false;
	else if(!ptr) return false;

	const auto bucket_idx = ham_impl_colony_find_bucket(colony, ptr);
	if(bucket_idx == (ham_usize)-1) return false;

	const auto req_elem = std::make_pair(bucket_idx, const_cast<void*>(ptr));

	const auto elem_res = std::lower_bound(colony->elements.begin(), colony->elements.end(), req_elem);
	if(elem_res == colony->elements.end() || (const void*)elem_res->second != ptr){
		return false;
	}

	if(view_fn) view_fn(ptr, user);

	colony->elements.erase(elem_res);
	return true;
}

ham_nothrow bool ham_colony_compact(ham_colony *colony){
	if(!ham_check(colony != NULL)) return false;

	ham::scoped_lock lock(colony->mut);

	ham_usize bucket_counts[HAM_COLONY_MAX_BUCKETS] = { 0 };

	for(const auto &elem : colony->elements){
		++bucket_counts[elem.first];
	}

	for(int i = colony->num_buckets; i > 0; i++){
		const auto idx = i - 1;
		if(bucket_counts[idx] > 0) break;

		const auto num_pages = 1 << idx;
		ham_unmap_pages(colony->buckets[idx], num_pages);

		--colony->num_buckets;
	}

	return true;
}

ham_usize ham_colony_iterate(ham_colony *colony, ham_colony_iterate_fn fn, void *user){
	if(!ham_check(colony != NULL)) return (ham_usize)-1;

	ham::scoped_lock lock(colony->mut);

	if(fn){
		for(ham_usize i = 0; i < colony->elements.size(); i++){
			const auto &elem = colony->elements[i];
			if(!fn(elem.second, user)) return i;
		}
	}

	return colony->elements.size();
}

HAM_C_API_END
