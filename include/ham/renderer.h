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

#ifndef HAM_RENDERER_H
#define HAM_RENDERER_H 1

/**
 * @defgroup HAM_RENDERER Rendering
 * @ingroup HAM
 * @{
 */

#include "shape.h"
#include "camera.h"
#include "vk.h" // IWYU pragma: keep

#include <stdarg.h>

HAM_C_API_BEGIN

typedef void*(*ham_gl_get_proc_addr)(const char *name);

typedef struct ham_renderer_create_args_vulkan{
	VkInstance instance;
	VkSurfaceKHR surface;
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
} ham_renderer_create_args_vulkan;

typedef struct ham_renderer_create_args_gl{
	ham_uptr context_handle;
	ham_gl_get_proc_addr glGetProcAddr;
} ham_renderer_create_args_gl;

typedef union ham_renderer_create_args{
	ham_renderer_create_args_vulkan vulkan;
	ham_renderer_create_args_gl gl;
} ham_renderer_create_args;

typedef struct ham_renderer_frame_data_common{
	ham_u64 current_frame;
	const ham_camera *cam;
} ham_renderer_frame_data_common;

typedef struct ham_renderer_frame_data_vulkan{
	ham_renderer_frame_data_common common;
	VkCommandBuffer command_buffer;
	VkFramebuffer framebuffer;
	VkRenderPass default_render_pass;
} ham_renderer_frame_data_vulkan;

typedef struct ham_renderer_frame_data_gl{
	ham_renderer_frame_data_common common;
} ham_renderer_frame_data_gl;

typedef union ham_renderer_frame_data{
	ham_renderer_frame_data_common common;
	ham_renderer_frame_data_vulkan vulkan;
	ham_renderer_frame_data_gl gl;
} ham_renderer_frame_data;

typedef struct ham_renderer ham_renderer;
typedef struct ham_renderer_vtable ham_renderer_vtable;

ham_api ham_renderer *ham_renderer_vptr_create(const ham_renderer_vtable *vptr, const ham_renderer_create_args *args);

ham_api ham_renderer *ham_renderer_create(const char *plugin_id, const char *obj_id, const ham_renderer_create_args *args);

ham_api void ham_renderer_destroy(ham_renderer *renderer);

ham_api bool ham_renderer_resize(ham_renderer *renderer, ham_u32 w, ham_u32 h);

ham_api void ham_renderer_frame(ham_renderer *renderer, ham_f64 dt, const ham_renderer_frame_data *data);

/**
 * @defgroup HAM_RENDERER_TEXTURE Textures
 * @{
 */

typedef struct ham_texture ham_texture;

/**
 * @}
 */

/**
 * @defgroup HAM_RENDERER_DRAW_GROUP Draw groups
 * @{
 */

typedef struct ham_draw_group ham_draw_group;

// typedef struct ham_draw

typedef struct ham_draw_group_instance_data{
	ham_mat4 trans;
	ham_vec4 color;
} ham_draw_group_instance_data;

ham_api ham_draw_group *ham_draw_group_create(
	ham_renderer *r,
	ham_usize num_shapes, const ham_shape *const *shapes
);

ham_api void ham_draw_group_destroy(ham_draw_group *group);

ham_api bool ham_draw_group_set_num_instances(ham_draw_group *group, ham_u32 n);

typedef bool(*ham_draw_group_instance_iterate_fn)(ham_draw_group_instance_data *data, void *user);

ham_api ham_u32 ham_draw_group_instance_iterate(
	ham_draw_group *group,
	ham_draw_group_instance_iterate_fn fn,
	void *user
);

ham_api bool ham_draw_group_instance_visit(
	ham_draw_group *group, ham_u32 idx,
	ham_draw_group_instance_iterate_fn fn,
	void *user
);

ham_api ham_u32 ham_draw_group_num_instances(const ham_draw_group *group);

/**
 * @}
 */

HAM_C_API_END

namespace ham{
	class renderer_exception: public exception{};

	class renderer_ctor_error: public renderer_exception{
		public:
			const char *api() const noexcept override{ return "ham::renderer::renderer"; }
			const char *what() const noexcept override{ return "Error in ham_renderer_create"; }
	};

	class renderer;

	template<typename ... Tags>
	class basic_renderer_view{
		public:
			constexpr static bool is_mutable = meta::type_list_contains_v<meta::type_list<Tags...>, mutable_tag>;

			using pointer = std::conditional_t<is_mutable, ham_renderer*, const ham_renderer*>;

			basic_renderer_view(pointer ptr_ = nullptr) noexcept
				: m_ptr(ptr_){}

			template<std::same_as<renderer> Renderer>
			basic_renderer_view(Renderer &r) noexcept
				: m_ptr(r.ptr()){}

			template<std::same_as<renderer> Renderer>
			basic_renderer_view(const Renderer &r) noexcept requires (is_mutable)
				: m_ptr(r.ptr()){}

			operator bool() const noexcept{ return !!m_ptr; }

			operator pointer() const noexcept{ return m_ptr; }

			bool resize(u32 w, u32 h) noexcept requires is_mutable{
				return ham_renderer_resize(m_ptr, w, h);
			}

			void frame(f64 dt, const ham_renderer_frame_data *data) requires is_mutable{
				ham_renderer_frame(m_ptr, dt, data);
			}

			pointer ptr() const noexcept{ return m_ptr; }

		private:
			pointer m_ptr;
	};

	using renderer_view = basic_renderer_view<mutable_tag>;
	using const_renderer_view = basic_renderer_view<>;

	class renderer{
		public:
			renderer() noexcept = default;

			renderer(const char *plugin_id, const char *object_id, const ham_renderer_create_args *create_args)
				: m_handle(ham_renderer_create(plugin_id, object_id, create_args))
			{
				if(!m_handle){
					throw renderer_ctor_error();
				}
			}

			static renderer make_from(ham_renderer *r) noexcept{
				return renderer(r);
			}

			operator bool() const noexcept{ return !!m_handle; }

			operator renderer_view() noexcept{ return m_handle.get(); }
			operator const_renderer_view() const noexcept{ return m_handle.get(); }

			bool resize(u32 w, u32 h) noexcept{ return ham_renderer_resize(m_handle.get(), w, h); }

			void frame(f64 dt, const ham_renderer_frame_data *data){ ham_renderer_frame(m_handle.get(), dt, data); }

			ham_renderer *ptr() noexcept{ return m_handle.get(); }
			const ham_renderer *ptr() const noexcept{ return m_handle.get(); }

		private:
			explicit renderer(ham_renderer *r) noexcept
				: m_handle(r){}

			unique_handle<ham_renderer*, ham_renderer_destroy> m_handle;
	};

	class draw_group_exception: public renderer_exception{};

	class draw_group_ctor_error: public draw_group_exception{
		public:
			const char *api() const noexcept override{ return "ham::draw_group::draw_group"; }
			const char *what() const noexcept override{ return "Error in ham_draw_group_create"; }
	};

	class draw_group;

	template<typename ... Tags>
	class basic_draw_group_view{
		public:
			constexpr static bool is_mutable = meta::type_list_contains_v<meta::type_list<Tags...>, mutable_tag>;

			using pointer = std::conditional_t<is_mutable, ham_draw_group*, const ham_draw_group*>;

			basic_draw_group_view(pointer ptr_ = nullptr) noexcept
				: m_ptr(ptr_){}

			template<std::same_as<draw_group> Group>
			basic_draw_group_view(Group &group) noexcept
				: m_ptr(group.ptr()){}

			template<std::same_as<draw_group> Group>
			basic_draw_group_view(const Group &group) noexcept requires (!is_mutable)
				: m_ptr(group.ptr()){}

			operator bool() const noexcept{ return !!m_ptr; }

			operator pointer() const noexcept{ return m_ptr; }

			pointer ptr() const noexcept{ return m_ptr; }

			bool set_num_instances(u32 n)
				requires (is_mutable)
			{
				return ham_draw_group_set_num_instances(m_ptr, n);
			}

			u32 num_instances() const noexcept{ return ham_draw_group_num_instances(m_ptr); }

			u32 instance_iterate(ham_draw_group_instance_iterate_fn iterate_fn, void *user)
				requires is_mutable
			{
				return ham_draw_group_instance_iterate(m_ptr, iterate_fn, user);
			}

			bool instance_visit(u32 idx, ham_draw_group_instance_iterate_fn view_fn, void *user)
				requires is_mutable
			{
				return ham_draw_group_instance_visit(m_ptr, idx, view_fn, user);
			}

		private:
			pointer m_ptr;
	};

	using draw_group_view = basic_draw_group_view<mutable_tag>;
	using const_draw_group_view = basic_draw_group_view<>;

	class draw_group{
		public:
			draw_group() noexcept = default;

			draw_group(renderer_view r, usize num_shapes, const ham_shape *const *shapes)
				: m_handle(ham_draw_group_create(r, num_shapes, shapes))
			{
				if(!m_handle){
					throw draw_group_ctor_error();
				}
			}

			static draw_group make_from(ham_draw_group *group) noexcept{
				return draw_group(group);
			}

			operator bool() const noexcept{ return !!m_handle; }

			operator draw_group_view() noexcept{ return m_handle.get(); }
			operator const_draw_group_view() const noexcept{ return m_handle.get(); }

			ham_draw_group *ptr() noexcept{ return m_handle.get(); }
			const ham_draw_group *ptr() const noexcept{ return m_handle.get(); }

			bool set_num_instances(u32 n){ return ham_draw_group_set_num_instances(m_handle.get(), n); }

			u32 num_instances() const noexcept{ return ham_draw_group_num_instances(m_handle.get()); }

			u32 instance_iterate(ham_draw_group_instance_iterate_fn iterate_fn, void *user){
				return ham_draw_group_instance_iterate(m_handle.get(), iterate_fn, user);
			}

			bool instance_visit(u32 idx, ham_draw_group_instance_iterate_fn view_fn, void *user){
				return ham_draw_group_instance_visit(m_handle.get(), idx, view_fn, user);
			}

		private:
			explicit draw_group(ham_draw_group *group) noexcept
				: m_handle(group){}

			unique_handle<ham_draw_group*, ham_draw_group_destroy> m_handle;
	};

	template<typename T>
		requires std::same_as<std::remove_cvref_t<T>, ham_draw_group*> || (std::is_class_v<std::remove_cvref_t<T>>)
	static inline bool draw_group_instance_iterate(T &&group, ham_draw_group_instance_iterate_fn iterate_fn, void *user){
		using group_type = std::remove_reference_t<T>;
		if constexpr(std::is_same_v<group_type, ham_draw_group*>){
			return ham_draw_group_instance_iterate(group, iterate_fn, user);
		}
		else{
			return std::forward<T>(group).instance_iterate(iterate_fn, user);
		}
	}

	template<typename T, typename Fn>
		requires std::invocable<Fn, ham_draw_group_instance_data*>
	static inline bool draw_group_instance_iterate(T &&group, Fn &&fn)
		noexcept(noexcept(std::forward<Fn>(fn)(std::declval<ham_draw_group_instance_data*>())))
	{
		static constexpr auto dispatcher = +[](ham_draw_group_instance_data *data, void *user) -> bool{
			const auto fn = reinterpret_cast<Fn*>(user);
			if constexpr(std::same_as<std::invoke_result_t<Fn, ham_draw_group_instance_data*>, void>){
				(*fn)(data);
				return true;
			}
			else{
				return (*fn)(data);
			}
		};

		return draw_group_instance_iterate(std::forward<T>(group), dispatcher, &fn);
	}

	template<typename T>
		requires std::same_as<std::remove_cvref_t<T>, ham_draw_group*> || (std::is_class_v<std::remove_cvref_t<T>>)
	static inline bool draw_group_instance_visit(T &&group, u32 idx, ham_draw_group_instance_iterate_fn visit_fn, void *user){
		using group_type = std::remove_reference_t<T>;
		if constexpr(std::is_same_v<group_type, ham_draw_group*>){
			return ham_draw_group_instance_visit(group, idx, visit_fn, user);
		}
		else{
			return std::forward<T>(group).instance_visit(idx, visit_fn, user);
		}
	}

	template<typename T, typename Fn>
		requires std::invocable<Fn, ham_draw_group_instance_data*>
	static inline bool draw_group_instance_visit(T &&group, u32 idx, Fn &&fn)
		noexcept(noexcept(std::forward<Fn>(fn)(std::declval<ham_draw_group_instance_data*>())))
	{
		static constexpr auto dispatcher = +[](ham_draw_group_instance_data *data, void *user) -> bool{
			const auto fn = reinterpret_cast<Fn*>(user);
			if constexpr(std::same_as<std::invoke_result_t<Fn, ham_draw_group_instance_data*>, void>){
				(*fn)(data);
				return true;
			}
			else{
				return (*fn)(data);
			}
		};

		return draw_group_instance_visit(std::forward<T>(group), idx, dispatcher, &fn);
	}
}

/**
 * @}
 */

#endif // !HAM_RENDERER_H
