#include "ham/renderer-object.h"
#include "ham/check.h"

HAM_C_API_BEGIN

ham_renderer *ham_renderer_vcreate(const char *plugin_id, const char *object_id, ham_usize nargs, va_list va){
	if(!ham_check(object_id != NULL)) return nullptr;

	ham_plugin *plugin = nullptr;
	ham_dso_handle dso = nullptr;

	if(!ham_plugin_find(plugin_id, HAM_EMPTY_STR8, &plugin, &dso)){
		ham_logapierrorf("Could not find plugin by id '%s'", plugin_id);
		return nullptr;
	}

	const auto obj_vtable = ham_plugin_object(plugin, ham::str8(object_id));
	if(!obj_vtable){
		ham_logapierrorf("Could not find object by id '%s'", object_id);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	const auto obj_info = obj_vtable->info();

	const auto allocator = ham_current_allocator();

	const auto obj = (ham_object*)ham_allocator_alloc(allocator, obj_info->alignment, obj_info->size);
	if(!obj){
		ham_logapierrorf("Error allocating memory for renderer object '%s'", object_id);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	if(!ham_plugin_init(plugin)){
		ham_logapierrorf("Error initializing plugin '%s'", ham_plugin_name(plugin).ptr);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	obj->vtable = obj_vtable;
	((ham_renderer*)obj)->allocator = allocator;
	((ham_renderer*)obj)->dso       = dso;
	((ham_renderer*)obj)->plugin    = plugin;

	const auto ret = (ham_renderer*)obj_vtable->construct(obj, nargs, va);
	if(!ret){
		ham_logapierrorf("Failed to construct renderer object '%s'", object_id);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	const auto draw_group_vtable = (const ham_object_vtable*)((const ham_renderer_vtable*)obj_vtable)->draw_group_vtable();

	ret->draw_groups = ham_object_manager_create(draw_group_vtable);
	if(!ret->draw_groups){
		ham_logapierrorf("Failed to create object manager for draw groups");
		obj_vtable->destroy(obj);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	const auto renderer_vt = (const ham_renderer_vtable*)obj_vtable;
	if(!renderer_vt->init(ret)){
		ham_logapierrorf("Failed to initialize renderer object '%s'", object_id);
		ham_object_manager_destroy(ret->draw_groups);
		obj_vtable->destroy(obj);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	return ret;
}

void ham_renderer_destroy(ham_renderer *r){
	if(ham_unlikely(!r)) return;

	const auto allocator = r->allocator;
	const auto obj_vt = ham_super(r)->vtable;
	const auto renderer_vt = (const ham_renderer_vtable*)obj_vt;

	const auto plugin = r->plugin;
	const auto dso = r->dso;

	ham_object_manager_iterate(
		r->draw_groups,
		[](ham_object *obj, void*){
			const auto vtable = (const ham_draw_group_vtable*)obj->vtable;
			const auto group = (ham_draw_group*)obj;
			vtable->fini(group);
			return true;
		},
		nullptr
	);

	ham_object_manager_destroy(r->draw_groups);

	renderer_vt->fini(r);

	obj_vt->destroy(ham_super(r));

	ham_allocator_free(allocator, r);

	ham_plugin_unload(plugin);
	ham_dso_close(dso);
}

void ham_renderer_loop(ham_renderer *renderer, ham_f64 dt){
	if(ham_unlikely(!renderer)) return;

	const auto renderer_vt = (const ham_renderer_vtable*)ham_super(renderer)->vtable;

	renderer_vt->loop(renderer, dt);
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

	const auto obj = ham_object_new(r->draw_groups);
	if(!obj){
		ham_logapierrorf("Failed to allocate new draw group");
		return nullptr;
	}

	const auto allocator = r->allocator;
	const auto vtable = obj->vtable;
	const auto ptr = (ham_draw_group*)obj;

	ptr->r = r;
	ptr->num_shapes = num_shapes;

	ptr->num_shape_points = (ham_usize*)ham_allocator_alloc(allocator, alignof(ham_usize), sizeof(ham_usize) * num_shapes);
	if(!ptr->num_shape_points){
		ham_logapierrorf("Failed to allocate memory for draw group data");
		ham_object_delete(r->draw_groups, obj);
		return nullptr;
	}

	ptr->num_shape_indices = (ham_usize*)ham_allocator_alloc(allocator, alignof(ham_usize), sizeof(ham_usize) * num_shapes);
	if(!ptr->num_shape_indices){
		ham_logapierrorf("Failed to allocate memory for draw group data");
		ham_allocator_free(allocator, ptr->num_shape_points);
		ham_object_delete(r->draw_groups, obj);
		return nullptr;
	}

	for(ham_usize i = 0; i < num_shapes; i++){
		ptr->num_shape_points[i] = ham_shape_num_points(shapes[i]);
		ptr->num_shape_indices[i] = ham_shape_num_indices(shapes[i]);
	}

	if(!((const ham_draw_group_vtable*)vtable)->init(ptr, r, num_shapes, shapes)){
		ham_logapierrorf("Failed to initialize draw group");
		ham_allocator_free(allocator, ptr->num_shape_indices);
		ham_allocator_free(allocator, ptr->num_shape_points);
		ham_object_delete(r->draw_groups, obj);
		return nullptr;
	}

	return ptr;
}

void ham_draw_group_destroy(ham_draw_group *group){
	if(ham_unlikely(group == NULL)) return;

	const auto vtable    = (const ham_draw_group_vtable*)ham_super(group)->vtable;
	const auto allocator = group->r->allocator;

	vtable->fini(group);

	ham_allocator_free(allocator, group->num_shape_indices);
	ham_allocator_free(allocator, group->num_shape_points);

	ham_object_delete(group->r->draw_groups, ham_super(group));
}

HAM_C_API_END
