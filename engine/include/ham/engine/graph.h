/*
 * Ham World Engine Runtime
 * Copyright (C) 2022 Keith Hammond
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HAME_ENGINE_GRAPH_H
#define HAME_ENGINE_GRAPH_H 1

/**
 * @defgroup HAM_ENGINE_GRAPH Node graphs
 * @{
 */

#include "ham/engine/config.h"
#include "ham/typedefs.h"
#include "ham/typesys.h"

HAM_C_API_BEGIN

typedef struct ham_graph ham_graph;
typedef struct ham_graph_node ham_graph_node;
typedef struct ham_graph_node_pin ham_graph_node_pin;
typedef struct ham_graph_node_connection ham_graph_node_connection;

ham_engine_api ham_graph *ham_graph_create(ham_str8 name);
ham_engine_api void ham_graph_destroy(ham_graph *graph);

ham_engine_api ham_str8 ham_graph_name(const ham_graph *graph);

ham_engine_api ham_usize ham_graph_num_nodes(const ham_graph *graph);

ham_engine_api ham_graph_node *const *ham_graph_nodes(ham_graph *graph);
ham_engine_api const ham_graph_node *const *ham_graph_const_nodes(const ham_graph *graph);

/**
 * @defgroup HAM_ENGINE_GRAPH_NODE Nodes
 * @{
 */

ham_engine_api ham_graph_node *ham_graph_node_create(ham_graph *graph, ham_str8 name);
ham_engine_api void ham_graph_node_destroy(ham_graph_node *node);

ham_engine_api ham_usize ham_graph_node_num_pins(const ham_graph_node *node);

ham_engine_api ham_graph_node_pin *const *ham_graph_node_pins(ham_graph_node *node);
ham_engine_api const ham_graph_node_pin *const *ham_graph_node_const_pins(const ham_graph_node *node);

/**
 * @}
 */

/**
 * @defgroup HAM_ENGINE_GRAPH_NODE_PIN Node pins
 * @{
 */

ham_engine_api ham_graph_node_pin *ham_graph_node_pin_create(ham_graph_node *node, ham_str8 name, const ham_type *type);
ham_engine_api void ham_graph_node_pin_destroy(ham_graph_node_pin *pin);

ham_engine_api ham_str8 ham_graph_node_pin_name(const ham_graph_node_pin *pin);
ham_engine_api const ham_type *ham_graph_node_pin_type(const ham_graph_node_pin *pin);

/**
 * @}
 */

typedef struct ham_graph_connection{
	ham_graph_node_pin *in, *out;
} ham_graph_connection;

ham_nonnull_args(1, 2) ham_used
ham_nothrow static inline ham_graph_connection ham_make_graph_connection(ham_graph_node_pin *in, ham_graph_node_pin *out){
	return (ham_graph_connection){ in, out };
}

HAM_C_API_END

#ifdef __cplusplus

namespace ham::engine{
	template<typename ... Tags>
	class basic_graph_node_pin_view{
		public:
			static constexpr bool is_mutable = meta::type_list_contains_v<meta::type_list<Tags...>, mutable_tag>;

			using pointer = std::conditional_t<is_mutable, ham_graph_node_pin*, const ham_graph_node_pin*>;

			basic_graph_node_pin_view(pointer ptr_ = nullptr) noexcept: m_ptr(ptr_){}

			operator pointer() const noexcept{ return m_ptr; }

			pointer ptr() const noexcept{ return m_ptr; }

			str8 name() const noexcept{ return ham_graph_node_pin_name(m_ptr); }
			ham::type type() const noexcept{ return ham_graph_node_pin_type(m_ptr); }

		private:
			pointer m_ptr;
	};

	using graph_node_pin_view = basic_graph_node_pin_view<mutable_tag>;
	using const_graph_node_pin_view = basic_graph_node_pin_view<>;

	template<typename ... Tags>
	class basic_graph_node_pin_array_view{
		public:
			static constexpr bool is_mutable = meta::type_list_contains_v<meta::type_list<Tags...>, mutable_tag>;

			using pointer = std::conditional_t<is_mutable, ham_graph_node_pin* const*, const ham_graph_node_pin* const*>;
			using element_view = std::conditional_t<is_mutable, graph_node_pin_view, const_graph_node_pin_view>;

			basic_graph_node_pin_array_view() noexcept: m_len(0), m_ptr(nullptr){}

			basic_graph_node_pin_array_view(usize len_, pointer ptr_) noexcept: m_len(len_), m_ptr(ptr_){}

			operator pointer() const noexcept{ return m_ptr; }

			pointer ptr() const noexcept{ return m_ptr; }

			usize size() const noexcept{ return m_len; }

			element_view operator[](usize idx) const noexcept{ return m_ptr[idx]; }

		private:
			usize m_len;
			pointer m_ptr;
	};

	using graph_node_pin_array_view = basic_graph_node_pin_array_view<mutable_tag>;
	using const_graph_node_pin_array_view = basic_graph_node_pin_array_view<>;

	template<typename ... Tags>
	class basic_graph_node_view{
		public:
			static constexpr bool is_mutable = meta::type_list_contains_v<meta::type_list<Tags...>, mutable_tag>;

			using pointer = std::conditional_t<is_mutable, ham_graph_node*, const ham_graph_node*>;
			using pin_array_view = std::conditional_t<is_mutable, graph_node_pin_array_view, const_graph_node_pin_array_view>;

			basic_graph_node_view(pointer ptr_ = nullptr) noexcept: m_ptr(ptr_){}

			operator pointer() const noexcept{ return m_ptr; }

			pointer ptr() const noexcept{ return m_ptr; }

			str8 name() const noexcept{ return ham_graph_node_name(m_ptr); }

			graph_node_pin_view create_pin(str8 name, type type_)
				requires is_mutable
			{
				return ham_graph_node_pin_create(m_ptr, name, type_);
			}

			void destroy_pin(graph_node_pin_view pin)
				requires is_mutable
			{
				ham_graph_node_pin_destroy(pin);
			}

			usize num_pins() const noexcept{ return ham_graph_node_num_pins(m_ptr); }

			pin_array_view pins() noexcept{
				if constexpr(is_mutable){
					return pin_array_view(num_pins(), ham_graph_node_pins(m_ptr));
				}
				else{
					return pin_array_view(num_pins(), ham_graph_node_const_pins(m_ptr));
				}
			}

		private:
			pointer m_ptr;
	};

	using graph_node_view = basic_graph_node_view<mutable_tag>;
	using const_graph_node_view = basic_graph_node_view<>;

	template<typename ... Tags>
	class basic_graph_node_array_view{
		public:
			static constexpr bool is_mutable = meta::type_list_contains_v<meta::type_list<Tags...>, mutable_tag>;

			using pointer = std::conditional_t<is_mutable, ham_graph_node* const*, const ham_graph_node* const*>;
			using element_view = std::conditional_t<is_mutable, graph_node_view, const_graph_node_view>;

			basic_graph_node_array_view() noexcept: m_len(0), m_ptr(nullptr){}

			basic_graph_node_array_view(usize len_, pointer ptr_) noexcept: m_len(len_), m_ptr(ptr_){}

			operator pointer() const noexcept{ return m_ptr; }

			pointer ptr() const noexcept{ return m_ptr; }

			usize size() const noexcept{ return m_len; }

			element_view operator[](usize idx) const noexcept{ return m_ptr[idx]; }

		private:
			usize m_len;
			pointer m_ptr;
	};

	using graph_node_array_view = basic_graph_node_array_view<mutable_tag>;
	using const_graph_node_array_view = basic_graph_node_array_view<>;

	template<typename ... Tags>
	class basic_graph_view{
		public:
			static constexpr bool is_mutable = meta::type_list_contains_v<meta::type_list<Tags...>, mutable_tag>;

			using pointer = std::conditional_t<is_mutable, ham_graph*, const ham_graph*>;

			basic_graph_view(pointer ptr_ = nullptr) noexcept: m_ptr(ptr_){}

			operator pointer() const noexcept{ return m_ptr; }

			pointer ptr() const noexcept{ return m_ptr; }

			str8 name() const noexcept{ return ham_graph_name(m_ptr); }

			usize num_nodes() const noexcept{ return ham_graph_num_nodes(m_ptr); }

			graph_node_view create_node(str8 name) const
				requires is_mutable
			{
				return ham_graph_node_create(m_ptr, name);
			}

			void destroy_node(graph_node_view node) const noexcept
				requires is_mutable
			{
				return ham_graph_node_destroy(node);
			}

		private:
			pointer m_ptr;
	};

	using graph_view = basic_graph_view<mutable_tag>;
	using const_graph_view = basic_graph_view<>;

	class graph{
		public:
			explicit graph(str8 name)
				: m_handle(ham_graph_create(name))
			{}

			operator graph_view() noexcept{ return m_handle.get(); }
			operator const_graph_view() const noexcept{ return m_handle.get(); }

			str8 name() const noexcept{ return ham_graph_name(m_handle.get()); }

			graph_node_view create_node(str8 name){
				return ham_graph_node_create(m_handle.get(), name);
			}

			void destroy_node(graph_node_view node) noexcept{
				ham_graph_node_destroy(node);
			}

			usize num_nodes() const noexcept{ return ham_graph_num_nodes(m_handle.get()); }

			graph_node_array_view nodes() noexcept{ return graph_node_array_view(num_nodes(), ham_graph_nodes(m_handle.get())); }
			const_graph_node_array_view nodes() const noexcept{ return const_graph_node_array_view(num_nodes(), ham_graph_const_nodes(m_handle.get())); }

		private:
			unique_handle<ham_graph*, ham_graph_destroy> m_handle;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // HAME_ENGINE_GRAPH_H
