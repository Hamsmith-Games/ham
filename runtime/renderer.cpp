/*
 * Ham Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
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

#include "ham/renderer-object.h"
#include "ham/check.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

static inline ham_renderer *ham_impl_renderer_construct(const ham_object_vtable *vtable, ham_object *obj, ...){
	va_list va;
	va_start(va, obj);
	const auto ret = vtable->ctor(obj, 1, va);
	va_end(va);
	return (ham_renderer*)ret;
}

ham_renderer *ham_renderer_create(const char *plugin_id, const char *obj_id, const ham_renderer_create_args *args){
	if(!ham_check(obj_id != NULL)) return nullptr;

	ham_plugin *plugin = nullptr;
	ham_dso_handle dso = nullptr;

	if(!ham_plugin_find(plugin_id, HAM_EMPTY_STR8, &plugin, &dso)){
		ham::logapierror("Could not find plugin by id '{}'", plugin_id);
		return nullptr;
	}

	const auto obj_vtable = ham_plugin_object(plugin, ham::str8(obj_id));
	if(!obj_vtable){
		ham::logapierror("Could not find object by id '{}'", obj_id);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	const auto obj_info = obj_vtable->info;

	const auto allocator = ham_current_allocator();

	const auto obj = (ham_object*)ham_allocator_alloc(allocator, obj_info->alignment, obj_info->size);
	if(!obj){
		ham::logapierror("Error allocating memory for renderer object '{}'", obj_id);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	if(!ham_plugin_init(plugin)){
		ham::logapierror("Error initializing plugin '{}'", ham_plugin_name(plugin).ptr);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	const auto r = (ham_renderer*)obj;

	ham_mutex draw_mut;
	if(!ham_mutex_init(&draw_mut, HAM_MUTEX_NORMAL)){
		ham::logapierror("Error in ham_mutex_init");
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	const auto draw_group_vtable = (const ham_object_vtable*)((const ham_renderer_vtable*)obj_vtable)->draw_group_vtable();

	const auto draw_group_manager = ham_object_manager_create(draw_group_vtable);
	if(!draw_group_manager){
		ham::logapierror("Failed to create object manager for draw groups");
		ham_mutex_finish(&draw_mut);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	ham_buffer draw_list, tmp_list;
	if(!ham_buffer_init_allocator(&draw_list, allocator, alignof(ham_draw_group*), 0)){
		ham::logapierror("Failed to initialize draw list: error in ham_buffer_init");
		ham_object_manager_destroy(draw_group_manager);
		ham_mutex_finish(&draw_mut);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	if(!ham_buffer_init_allocator(&tmp_list, allocator, alignof(ham_draw_group*), 0)){
		ham::logapierror("Failed to initialize draw temp list: error in ham_buffer_init");
		ham_buffer_finish(&draw_list);
		ham_object_manager_destroy(draw_group_manager);
		ham_mutex_finish(&draw_mut);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	obj->vptr    = obj_vtable;
	r->allocator   = allocator;
	r->dso         = dso;
	r->plugin      = plugin;
	r->draw_mut    = draw_mut;
	r->draw_groups = draw_group_manager;
	r->draw_list   = draw_list;
	r->tmp_list    = tmp_list;

	const auto ret = ham_impl_renderer_construct(obj->vptr, obj, args);
	if(!ret){
		ham::logapierror("Failed to construct renderer object '{}'", obj_id);
		ham_buffer_finish(&draw_list);
		ham_buffer_finish(&tmp_list);
		ham_object_manager_destroy(draw_group_manager);
		ham_mutex_finish(&draw_mut);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	// in case somebody did something weird in a constructor, we have to set these again
	// TODO: only do this in debug builds?
	obj->vptr      = obj_vtable;
	ret->allocator   = allocator;
	ret->dso         = dso;
	ret->plugin      = plugin;
	ret->draw_mut    = draw_mut;
	ret->draw_groups = draw_group_manager;

	// check we don't muck up the draw lists tho
	if(
	   ret->draw_list.allocator != allocator || ret->draw_list.alignment != alignof(ham_draw_group*) ||
	   ret->tmp_list.allocator  != allocator || ret->tmp_list.alignment  != alignof(ham_draw_group*)
	){
		// ... could still be bad tho ,':^)
		ham::logapiwarn("Draw lists changed in object constructor, resetting to initial");
		ret->draw_list = draw_list;
		ret->tmp_list  = tmp_list;
	}

	return ret;
}

void ham_renderer_destroy(ham_renderer *r){
	if(ham_unlikely(!r)) return;

	const auto allocator = r->allocator;
	const auto obj_vt = ham_super(r)->vptr;
	//const auto renderer_vt = (const ham_renderer_vtable*)obj_vt;

	const auto plugin = r->plugin;
	const auto dso = r->dso;

	ham_buffer_finish(&r->draw_list);
	ham_buffer_finish(&r->tmp_list);

	ham_object_manager_iterate(
		r->draw_groups,
		[](ham_object *obj, void*){
			const auto vtable = (const ham_draw_group_vtable*)obj->vptr;
			const auto group = (ham_draw_group*)obj;
			return true;
		},
		nullptr
	);

	ham_object_manager_destroy(r->draw_groups);

	ham_mutex_finish(&r->draw_mut);

	obj_vt->dtor(ham_super(r));

	ham_allocator_free(allocator, r);

	ham_plugin_unload(plugin);
	ham_dso_close(dso);
}

// bool ham_renderer_swapchain_reset(ham_renderer *r){
// 	if(!ham_check(r != NULL)) return false;
//
// 	const auto vtable = (const ham_renderer_vtable*)ham_super(r)->vtable;
// 	return vtable->swapchain_reset(r);
// }

bool ham_renderer_resize(ham_renderer *r, ham_u32 w, ham_u32 h){
	if(!ham_check(r != NULL) || !ham_check(w > 0) || !ham_check(h > 0)){
		return false;
	}

	const auto vptr = (const ham_renderer_vtable*)ham_super(r)->vptr;
	return vptr->resize(r, w, h);
}

void ham_renderer_frame(ham_renderer *renderer, ham_f64 dt, const ham_renderer_frame_data *data){
	if(ham_unlikely(!renderer) || !ham_check(data != NULL)) return;

	const auto renderer_vt = (const ham_renderer_vtable*)ham_super(renderer)->vptr;
	renderer_vt->frame(renderer, dt, data);
}

//
// Draw groups
//

ham_draw_group *ham_draw_group_create(
	ham_renderer *r,
	ham_usize num_shapes, const ham_shape *const *shapes
){
	if(
	   !ham_check(r != NULL) ||
	   !ham_check(num_shapes > 0) ||
	   !ham_check(shapes != NULL)
	){
		return nullptr;
	}

	ham_mutex mut;
	if(!ham_mutex_init(&mut, HAM_MUTEX_NORMAL)){
		ham::logapierror("Error in ham_mutex_init");
		return nullptr;
	}

	const auto allocator = r->allocator;

	const auto num_shape_points_arr = (ham_usize*)ham_allocator_alloc(allocator, alignof(ham_usize), sizeof(ham_usize) * num_shapes);
	if(!num_shape_points_arr){
		ham::logapierror("Failed to allocate memory for draw group point count data");
		ham_mutex_finish(&mut);
		return nullptr;
	}

	const auto num_shape_indices_arr = (ham_usize*)ham_allocator_alloc(allocator, alignof(ham_usize), sizeof(ham_usize) * num_shapes);
	if(!num_shape_indices_arr){
		ham::logapierror("Failed to allocate memory for draw group index count data");
		ham_allocator_free(allocator, num_shape_points_arr);
		ham_mutex_finish(&mut);
		return nullptr;
	}

	for(ham_usize i = 0; i < num_shapes; i++){
		num_shape_points_arr[i]  = ham_shape_num_points(shapes[i]);
		num_shape_indices_arr[i] = ham_shape_num_indices(shapes[i]);
	}

	struct draw_group_data{
		ham_renderer *r;
		ham_usize num_shapes;
		const ham_shape *const *shapes;
		ham_usize *num_shape_points_arr;
		ham_usize *num_shape_indices_arr;
	};

	draw_group_data data{r, num_shapes, shapes, num_shape_points_arr, num_shape_indices_arr};

	const auto obj = ham_object_new_init(
		r->draw_groups,
		[](ham_object *obj, void *user){
			const auto group = (ham_draw_group*)obj;
			const auto data  = (draw_group_data*)user;

			group->r = data->r;
			group->num_shapes = data->num_shapes;
			group->num_shape_points = data->num_shape_points_arr;
			group->num_shape_indices = data->num_shape_indices_arr;

			return true;
		},
		&data,
		num_shapes, shapes
	);
	if(!obj){
		ham::logapierror("Failed to construct new draw group");
		ham_allocator_free(allocator, num_shape_indices_arr);
		ham_allocator_free(allocator, num_shape_points_arr);
		ham_mutex_finish(&mut);
		return nullptr;
	}

	const auto ptr = (ham_draw_group*)obj;

	ptr->r = r;
	ptr->mut = mut;
	ptr->num_shapes = num_shapes;
	ptr->num_shape_points = num_shape_points_arr;
	ptr->num_shape_indices = num_shape_indices_arr;

	return ptr;
}

void ham_draw_group_destroy(ham_draw_group *group){
	if(ham_unlikely(group == NULL)) return;

	const auto allocator = group->r->allocator;

	ham_allocator_free(allocator, group->num_shape_indices);
	ham_allocator_free(allocator, group->num_shape_points);

	ham_mutex_finish(&group->mut);

	ham_object_delete(group->r->draw_groups, ham_super(group));
}

bool ham_draw_group_set_num_instances(ham_draw_group *group, ham_u32 n){
	if(!ham_check(group != NULL)) return false;

	if(!ham_mutex_lock(&group->mut)){
		ham::logapierror("Error in ham_mutex_lock");
		return false;
	}

	const auto vptr = (const ham_draw_group_vtable*)ham_super(group)->vptr;
	const bool result = vptr->set_num_instances(group, n);
	if(result){
		group->num_instances = n;
	}

	if(!ham_mutex_unlock(&group->mut)){
		ham::logapiwarn("Error in ham_mutex_unlock");
	}

	return result;
}

ham_u32 ham_draw_group_num_instances(const ham_draw_group *group){
	if(!ham_check(group != NULL)) return (ham_u32)-1;
	return group->num_instances;
}

ham_u32 ham_draw_group_instance_iterate(
	ham_draw_group *group,
	ham_draw_group_instance_iterate_fn fn,
	void *user
){
	if(!ham_check(group != NULL)) return (ham_u32)-1;

	if(!ham_mutex_lock(&group->mut)){
		ham::logapierror("Error in ham_mutex_lock");
		return (ham_u32)-1;
	}

	u32 result = group->num_instances;

	if(fn){
		const auto vptr = (const ham_draw_group_vtable*)ham_super(group)->vptr;
		const auto data = vptr->instance_data(group);

		const auto n = group->num_instances;

		for(result = 0; result < n; result++){
			const auto inst = data + result;
			if(!fn(inst, data)) break;
		}
	}

	if(!ham_mutex_unlock(&group->mut)){
		ham::logapiwarn("Error in ham_mutex_unlock");
	}

	return result;
}

bool ham_draw_group_instance_visit(
	ham_draw_group *group, ham_u32 idx,
	ham_draw_group_instance_iterate_fn fn,
	void *user
){
	if(!ham_check(group != NULL)) return false;

	if(!ham_mutex_lock(&group->mut)){
		ham::logapierror("Error in ham_mutex_lock");
		return false;
	}

	bool result = ham_check(idx < group->num_instances);
	if(result){
		const auto vptr = (const ham_draw_group_vtable*)ham_super(group)->vptr;
		const auto data = vptr->instance_data(group);
		result = fn(data + idx, user);
	}

	if(!ham_mutex_unlock(&group->mut)){
		ham::logapiwarn("Error in ham_mutex_unlock");
	}

	return result;
}

HAM_C_API_END
