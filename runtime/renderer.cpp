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

	ham_u8 default_tex_pixels[512*512*4];
	constexpr ham_color_u8 pixel_color = { .data = { 255, 120, 0, 255 } };

	for(ham_u32 y = 0; y < 512; y++){
		for(ham_u32 x = 0; x < 512; x++){
			const auto idx = ((y * 512) + x) * 4;
			memcpy(default_tex_pixels + idx, &pixel_color, sizeof(ham_color_u8));
		}
	}

	const auto default_img = ham_image_create(HAM_RGBA8U, 512, 512, default_tex_pixels);
	if(!default_img){
		ham::logapierror("Could not create default texture");
		ham_mutex_finish(&draw_mut);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	const auto draw_group_vtable = (const ham_object_vtable*)((const ham_renderer_vtable*)obj_vtable)->draw_group_vtable();
	const auto light_group_vtable = (const ham_object_vtable*)((const ham_renderer_vtable*)obj_vtable)->light_group_vtable();

	const auto draw_group_manager = ham_object_manager_create(draw_group_vtable);
	if(!draw_group_manager){
		ham::logapierror("Failed to create object manager for draw groups");
		ham_image_destroy(default_img);
		ham_mutex_finish(&draw_mut);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	const auto light_group_manager = ham_object_manager_create(light_group_vtable);
	if(!light_group_manager){
		ham::logapierror("Failed to create object manager for light groups");
		ham_object_manager_destroy(draw_group_manager);
		ham_image_destroy(default_img);
		ham_mutex_finish(&draw_mut);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	ham_buffer draw_list, tmp_list;
	if(!ham_buffer_init_allocator(&draw_list, allocator, alignof(ham_draw_group*), 0)){
		ham::logapierror("Failed to initialize draw list: error in ham_buffer_init");
		ham_object_manager_destroy(light_group_manager);
		ham_object_manager_destroy(draw_group_manager);
		ham_image_destroy(default_img);
		ham_mutex_finish(&draw_mut);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	if(!ham_buffer_init_allocator(&tmp_list, allocator, alignof(ham_draw_group*), 0)){
		ham::logapierror("Failed to initialize draw temp list: error in ham_buffer_init");
		ham_buffer_finish(&draw_list);
		ham_object_manager_destroy(light_group_manager);
		ham_object_manager_destroy(draw_group_manager);
		ham_image_destroy(default_img);
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
	r->default_img = default_img;
	r->draw_mut    = draw_mut;
	r->draw_groups = draw_group_manager;
	r->draw_list   = draw_list;
	r->tmp_list    = tmp_list;

	const auto ret = ham_impl_renderer_construct(obj->vptr, obj, args);
	if(!ret){
		ham::logapierror("Failed to construct renderer object '{}'", obj_id);
		ham_buffer_finish(&draw_list);
		ham_buffer_finish(&tmp_list);
		ham_object_manager_destroy(light_group_manager);
		ham_object_manager_destroy(draw_group_manager);
		ham_image_destroy(default_img);
		ham_mutex_finish(&draw_mut);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	// in case somebody did something weird in a constructor, we have to set these again
	// TODO: only do this in debug builds?
	obj->vptr         = obj_vtable;
	ret->allocator    = allocator;
	ret->dso          = dso;
	ret->default_img  = default_img;
	ret->plugin       = plugin;
	ret->draw_mut     = draw_mut;
	ret->draw_groups  = draw_group_manager;
	ret->light_groups = light_group_manager;

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
			(void)vtable; (void)group;
			return true;
		},
		nullptr
	);

	ham_object_manager_destroy(r->draw_groups);

	ham_image_destroy(r->default_img);

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

const ham_image *ham_renderer_default_image(const ham_renderer *renderer){
	if(!ham_check(renderer != NULL)) return nullptr;
	return renderer->default_img;
}

bool ham_renderer_add_shader_include(ham_renderer *r, ham_str8 name, ham_str8 src){
	if(
	   !ham_check(r != NULL) ||
	   !ham_check(name.len && name.ptr) ||
	   !ham_check(src.len && src.ptr) ||
	   !ham_check(name.len < (HAM_NAME_BUFFER_SIZE - 1))
	){
		return false;
	}

	const auto vt = (const ham_renderer_vtable*)ham_super(r)->vptr;
	return vt->add_shader_include(r, name, src);
}

//
// Shaders
//

static inline ham_shader *ham_shader_create_ctor(ham_shader *shader, ...){
	const auto vt  = ham_super(shader)->vptr;

	va_list va;
	va_start(va, shader);
	const auto obj = vt->ctor(ham_super(shader), 1, va);
	va_end(va);

	return (ham_shader*)obj;
}

ham_shader *ham_shader_create(ham_renderer *r, ham_shader_kind kind){
	if(!ham_check(r != NULL) || !ham_check(kind < HAM_SHADER_KIND_COUND)){
		return nullptr;
	}

	const auto vt = ham_super(r)->vptr;
	const auto vi = vt->info;

	const auto allocator = r->allocator;

	const auto shader_mem = ham_allocator_alloc(allocator, vi->alignment, vi->size);
	if(!shader_mem) return nullptr;

	const auto shader = new(shader_mem) ham_shader;

	ham_super(shader)->vptr = vt;

	shader->r    = r;
	shader->kind = kind;

	shader->compiled = false;

	const auto ret = ham_shader_create_ctor(shader, (u32)kind);
	if(!ret){
		ham::logapierror("Failed to construct shader of type '{}'", vi->type_id);
		ham_allocator_delete(allocator, shader);
		return nullptr;
	}

	return ret;
}

void ham_shader_destroy(ham_shader *shader){
	if(ham_unlikely(!shader)) return;

	const auto allocator = shader->r->allocator;

	const auto vt = ham_super(shader)->vptr;
	vt->dtor(ham_super(shader));

	std::destroy_at(shader);
	ham_allocator_free(allocator, shader);
}

bool ham_shader_set_source(ham_shader *shader, ham_shader_source_kind kind, ham_str8 src){
	if(!ham_check(shader != NULL) || !ham_check(src.len && src.ptr)){
		return false;
	}
	else if(shader->compiled){
		ham::logapierror("Shader already compiled");
		return false;
	}

	const auto vt = (const ham_shader_vtable*)ham_super(shader)->vptr;
	return vt->set_source(shader, kind, src);
}

bool ham_shader_compile(ham_shader *shader){
	if(!ham_check(shader != NULL)) return false;
	else if(shader->compiled){
		ham::logapiwarn("Shader already compiled");
		return true;
	}

	const auto vt = (const ham_shader_vtable*)ham_super(shader)->vptr;
	shader->compiled = vt->compile(shader);
	return shader->compiled;
}

//
// Draw groups
//

ham_draw_group *ham_draw_group_create(
	ham_renderer *r,
	ham_usize num_shapes, const ham_shape *const *shapes, const ham_image *const *images
){
	if(
	   !ham_check(r != NULL) ||
	   !ham_check(num_shapes > 0) ||
	   !ham_check(shapes != NULL) ||
	   !ham_check(images != NULL)
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

	const auto images_arr = (const ham_image**)ham_allocator_alloc(allocator, alignof(void*), sizeof(void*) * num_shapes);
	if(!images_arr){
		ham::logapierror("Failed to allocate memory for draw group image array");
		ham_allocator_free(allocator, num_shape_points_arr);
		ham_allocator_free(allocator, num_shape_indices_arr);
		ham_mutex_finish(&mut);
		return nullptr;
	}

	for(ham_usize i = 0; i < num_shapes; i++){
		num_shape_points_arr[i]  = ham_shape_num_points(shapes[i]);
		num_shape_indices_arr[i] = ham_shape_num_indices(shapes[i]);
		images_arr[i] = images[i];
	}

	struct draw_group_data{
		ham_renderer *r;
		ham_usize num_shapes;
		const ham_shape *const *shapes;
		ham_usize *num_shape_points_arr;
		ham_usize *num_shape_indices_arr;
		const ham_image **images;
	};

	draw_group_data data{r, num_shapes, shapes, num_shape_points_arr, num_shape_indices_arr, images_arr};

	const auto obj = ham_object_new_init(
		r->draw_groups,
		[](ham_object *obj, void *user){
			const auto group = (ham_draw_group*)obj;
			const auto data  = (draw_group_data*)user;

			group->r = data->r;
			group->num_shapes = data->num_shapes;
			group->num_shape_points = data->num_shape_points_arr;
			group->num_shape_indices = data->num_shape_indices_arr;
			group->images = data->images;

			return true;
		},
		&data,
		num_shapes, shapes, images_arr
	);
	if(!obj){
		ham::logapierror("Failed to construct new draw group");
		ham_allocator_free(allocator, num_shape_indices_arr);
		ham_allocator_free(allocator, num_shape_points_arr);
		ham_allocator_free(allocator, images_arr);
		ham_mutex_finish(&mut);
		return nullptr;
	}

	const auto ptr = (ham_draw_group*)obj;

	ptr->r = r;
	ptr->mut = mut;
	ptr->num_shapes = num_shapes;
	ptr->num_shape_points = num_shape_points_arr;
	ptr->num_shape_indices = num_shape_indices_arr;
	ptr->images = images_arr;

	return ptr;
}

void ham_draw_group_destroy(ham_draw_group *group){
	if(ham_unlikely(!group)) return;

	const auto allocator = group->r->allocator;

	ham_allocator_free(allocator, group->num_shape_indices);
	ham_allocator_free(allocator, group->num_shape_points);
	ham_allocator_free(allocator, group->images);

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

const ham_image *ham_draw_group_image(const ham_draw_group *group, ham_usize idx){
	if(!ham_check(group != NULL) || !ham_check(idx <= group->num_shapes)){
		return nullptr;
	}

	return group->images[idx];
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

//
// Light groups
//

ham_light_group *ham_light_group_create(ham_renderer *r, ham_usize num_lights){
	if(!ham_check(r != NULL)) return nullptr;

	struct light_group_data{
		ham_renderer *r;
		ham_mutex mut;
		ham_usize num_lights;
	};

	light_group_data data;
	data.r = r;
	data.num_lights = num_lights;

	if(!ham_mutex_init(&data.mut, HAM_MUTEX_NORMAL)){
		ham::logapierror("Error in ham_mutex_init");
		return nullptr;
	}

	const auto obj = ham_object_new_init(
		r->light_groups, [](ham_object *obj, void *user){
			const auto data = (const light_group_data*)user;
			const auto group = (ham_light_group*)obj;
			group->r = data->r;
			group->mut = data->mut;;
			group->num_instances = 0;
			return true;
		},
		&data
	);
	if(!obj){
		ham::logapierror("Failed to construct new light group");
		ham_mutex_finish(&data.mut);
		return nullptr;
	}

	const auto group = (ham_light_group*)obj;

	const auto vptr = (const ham_light_group_vtable*)ham_super(group)->vptr;
	const bool result = vptr->set_num_instances(group, num_lights);
	if(result){
		group->num_instances = num_lights;
	}
	else{
		ham::logapiwarn("Failed to set number of instances to {}", num_lights);
	}

	return (ham_light_group*)obj;
}

void ham_light_group_destroy(ham_light_group *group){
	if(ham_unlikely(!group)) return;

	ham_mutex_finish(&group->mut);

	ham_object_delete(group->r->light_groups, ham_super(group));
}

ham_u32 ham_light_group_instance_iterate(
	ham_light_group *group,
	ham_light_group_instance_iterate_fn fn,
	void *user
){
	if(!ham_check(group != NULL)) return (ham_u32)-1;

	if(!ham_mutex_lock(&group->mut)){
		ham::logapierror("Error in ham_mutex_lock");
		return (ham_u32)-1;
	}

	u32 result = group->num_instances;

	if(fn){
		const auto vptr = (const ham_light_group_vtable*)ham_super(group)->vptr;
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

bool ham_light_group_instance_visit(
	ham_light_group *group, ham_u32 idx,
	ham_light_group_instance_iterate_fn fn,
	void *user
){
	if(!ham_check(group != NULL)) return false;

	if(!ham_mutex_lock(&group->mut)){
		ham::logapierror("Error in ham_mutex_lock");
		return false;
	}

	bool result = ham_check(idx < group->num_instances);
	if(result){
		const auto vptr = (const ham_light_group_vtable*)ham_super(group)->vptr;
		const auto data = vptr->instance_data(group);
		result = fn(data + idx, user);
	}

	if(!ham_mutex_unlock(&group->mut)){
		ham::logapiwarn("Error in ham_mutex_unlock");
	}

	return result;
}

ham_u32 ham_light_group_num_instances(const ham_light_group *group){
	if(!ham_check(group != NULL)) return (ham_u32)-1;
	else return group->num_instances;
}

bool ham_light_group_set_num_instances(ham_light_group *group, ham_u32 n){
	if(!ham_check(group != NULL)) return false;

	if(!ham_mutex_lock(&group->mut)){
		ham::logapierror("Error in ham_mutex_lock");
		return false;
	}

	const auto vptr = (const ham_light_group_vtable*)ham_super(group)->vptr;
	const bool result = vptr->set_num_instances(group, n);
	if(result){
		group->num_instances = n;
	}
	else{
		ham::logapierror("Failed to set number of instances to {}", n);
	}

	if(!ham_mutex_unlock(&group->mut)){
		ham::logapiwarn("Error in ham_mutex_unlock");
	}

	return result;
}

HAM_C_API_END
