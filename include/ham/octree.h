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

#ifndef HAM_OCTREE_H
#define HAM_OCTREE_H 1

/**
 * @defgroup HAM_OCTREE Sparse Octrees
 * @ingroup HAM
 * @{
 */

#include "ham/typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_octree ham_octree;
typedef struct ham_octree_layer ham_octree_layer;
typedef struct ham_octree_node ham_octree_node;

ham_api ham_octree *ham_octree_create(ham_usize leaf_alignment, ham_usize leaf_size);

ham_api ham_nothrow void ham_octree_destroy(ham_octree *tree);

/**
 * @brief Get the root node of a tree
 * @param tree tree to get the root node of
 * @returns root node or ``NULL`` on error
 */
ham_api ham_nothrow ham_octree_node *ham_octree_root(ham_octree *tree);

ham_api ham_nothrow ham_usize ham_octree_current_depth(ham_octree *tree);

typedef bool(*ham_octree_iterate_fn)(ham_octree_layer *layer, void *user);

ham_api ham_usize ham_octree_iterate(ham_octree *tree, ham_octree_iterate_fn fn, void *user);

ham_api ham_octree_layer *ham_octree_layer_at(ham_octree *tree, ham_usize depth);

/**
 * @defgroup HAM_OCTREE_LAYER Layers
 * @{
 */

typedef bool(*ham_octree_layer_iterate_fn)(ham_octree_node *node, void *user);

ham_api ham_usize ham_octree_layer_iterate(ham_octree_layer *layer, ham_octree_layer_iterate_fn fn, void *user);

ham_api ham_nothrow ham_usize ham_octree_layer_num_nodes(const ham_octree_layer *layer);

ham_api ham_nothrow ham_octree_node *ham_octree_layer_node_at(ham_octree_layer *layer, ham_usize idx);

/**
 * @}
 */

/**
 * @defgroup HAM_OCTREE_NODE Nodes
 * @{
 */

ham_api ham_nothrow bool ham_octree_node_is_root(const ham_octree_node *node);

ham_api ham_nothrow bool ham_octree_node_is_leaf(const ham_octree_node *node);

ham_api ham_nothrow ham_octree_node *ham_octree_node_parent(ham_octree_node *node);

ham_api ham_nothrow ham_octree *ham_octree_node_tree(ham_octree_node *node);

ham_api ham_nothrow void *ham_octree_node_value(ham_octree_node *node);

/**
 * @brief Get a child node from it's parent.
 * @param node node to get child of
 * @param idx index of the child in range \f$([0,7])\f$
 * @returns child node or ``NULL`` on error
 */
ham_api ham_nothrow ham_octree_node *ham_octree_node_child(ham_octree_node *node, ham_usize idx);

/**
 * @brief Emplace an empty branch node in a given parent.
 * @param parent node to emplace into
 * @param idx index of the new node
 * @returns newly created node or ``NULL`` on error
 */
ham_api ham_octree_node *ham_octree_node_emplace(ham_octree_node *parent, ham_usize idx);

/**
 * @brief Emplace an empty leaf node in a given parent.
 * @param parent node to emplace into
 * @param idx index of the new node
 * @return newly created node or ``NULL`` on error
 */
ham_api ham_octree_node *ham_octree_node_emplace_leaf(ham_octree_node *parent, ham_usize idx);

/**
 * @brief Try to turn a node into a leaf node.
 * @param node node to convert
 * @pre \p node must not have any allocated children
 * @returns whether \p node was successfully turned into a leaf node
 */
ham_api bool ham_octree_node_make_leaf(ham_octree_node *node);

/**
 * @}
 */

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	template<typename T>
	class octree;

	template<typename T>
	class octree_view;

	template<typename T>
	class octree_layer;

	class octree_node_value_error{
		public:
			const char *api(){ return "octree_node::value"; }
			const char *what(){ return "failed to get reference to value"; }
	};

	template<typename T>
	class octree_node{
		public:
			using tree_view_type = octree_view<T>;

			operator bool() const noexcept{ return !!m_ptr; }

			bool is_leaf() const noexcept{ return ham_octree_node_is_leaf(m_ptr); }

			tree_view_type tree() noexcept{ return ham_octree_node_tree(m_ptr); }

			octree_node parent() noexcept{ return ham_octree_node_parent(m_ptr); }

			octree_node child(usize idx) const noexcept{ return ham_octree_node_child(m_ptr, idx); }

			octree_node emplace(usize idx) const{ return ham_octree_node_emplace(m_ptr, idx); }

			template<typename ... Args>
			octree_node emplace_leaf(usize idx, Args &&... args) const{
				const auto leaf_node = ham_octree_node_emplace_leaf(m_ptr, idx);
				if(!leaf_node) return nullptr;

				const auto leaf_ptr = ham_octree_node_value(leaf_node);
				new(leaf_ptr) T(std::forward<Args>(args)...);

				return leaf_node;
			}

			template<typename ... Args>
			bool make_leaf(Args &&... args) const{
				if(!ham_octree_node_make_leaf(m_ptr)){
					return false;
				}

				const auto ptr = ham_octree_node_value(m_ptr);
				new(ptr) T(std::forward<Args>(args)...);

				return true;
			}

			T *value_ptr() const{
				const auto ptr = ham_octree_node_value(m_ptr);
				return reinterpret_cast<T*>(ptr);
			}

			T &value() const{
				const auto val_ptr = value_ptr();
				if(!val_ptr){
					throw octree_node_value_error{};
				}

				return *val_ptr;
			}

		private:
			octree_node(ham_octree_node *ptr_)
				: m_ptr(ptr_){}

			ham_octree_node *m_ptr;

			friend class octree<T>;
			friend class octree_view<T>;
			friend class octree_layer<T>;
	};

	template<typename T>
	class octree_layer{
		public:
			using node_type = octree_node<T>;

			usize num_nodes() const noexcept{ return ham_octree_layer_num_nodes(m_ptr); }

			node_type node_at(usize idx) noexcept{ return ham_octree_layer_node_at(m_ptr, idx); }

			usize iterate(ham_octree_layer_iterate_fn fn, void *user){
				return ham_octree_layer_iterate(m_ptr, fn, user);
			}

			template<typename Fn>
			usize iterate(Fn &&fn) noexcept(noexcept(std::forward<Fn>(fn)(std::declval<node_type>()))){
				struct fn_wrapper{
					Fn *fn;
				} wrapped(&fn);

				constexpr auto dispatcher = +[](ham_octree_node *node, void *user) -> bool{
					const auto wrapped = reinterpret_cast<fn_wrapper*>(user);
					if constexpr(std::is_same_v<std::invoke_result_t<Fn, node_type>, void>){
						(*wrapped->fn)(node_type(node));
						return true;
					}
					else{
						return (*wrapped->fn)(node_type(node));
					}
				};

				return ham_octree_layer_iterate(m_ptr, dispatcher, &wrapped);
			}

		private:
			octree_layer(ham_octree_layer *ptr_) noexcept
				: m_ptr(ptr_){}

			ham_octree_layer *m_ptr;

			friend class octree<T>;
			friend class octree_view<T>;
	};

	template<typename T>
	class octree{
		public:
			using node_type = octree_node<T>;
			using layer_type = octree_layer<T>;

			octree()
				: m_handle(ham_octree_create(alignof(T), sizeof(T))){}

			octree(octree&&) noexcept = default;

			~octree(){
				if(!m_handle) return;

				// Destrot constructed values
				ham_octree_iterate(
					m_handle.get(),
					[](ham_octree_layer *layer, void*) -> bool{
						ham_octree_layer_iterate(
							layer,
							[](ham_octree_node *node, void*) -> bool{
								if(ham_octree_node_is_leaf(node)){
									const auto ptr = reinterpret_cast<T*>(ham_octree_node_value(node));
									std::destroy_at(ptr);
								}
								return true;
							},
							nullptr
						);
						return true;
					},
					nullptr
				);
			}

			node_type root() noexcept{ return ham_octree_root(m_handle.get()); }

			usize iterate(ham_octree_iterate_fn fn, void *user){
				return ham_octree_iterate(m_handle.get(), fn, user);
			}

			template<typename Fn>
			usize iterate(Fn &&fn) noexcept(noexcept(std::forward<Fn>(fn)(std::declval<layer_type>()))){
				struct fn_wrapper{
					Fn *fn;
				} wrapped(&fn);

				constexpr auto dispatcher = +[](ham_octree_layer *layer, void *user) -> bool{
					const auto wrapped = reinterpret_cast<fn_wrapper*>(user);
					if constexpr(std::is_same_v<std::invoke_result_t<Fn, layer_type>, void>){
						(*wrapped->fn)(layer_type(layer));
						return true;
					}
					else{
						return (*wrapped->fn)(layer_type(layer));
					}
				};

				return ham_octree_iterate(m_handle.get(), dispatcher, &wrapped);
			}

		private:
			unique_handle<ham_octree*, ham_octree_destroy> m_handle;

			friend class octree_view<T>;
	};

	template<typename T>
	class octree_view{
		public:
			using octree_type = octree<T>;
			using layer_type = octree_layer<T>;
			using node_type = octree_node<T>;

			octree_view() noexcept
				: m_ptr(nullptr){}

			octree_view(octree_type &tree)
				: m_ptr(tree.ptr()){}

			node_type root() noexcept{ return ham_octree_root(m_ptr); }

			usize iterate(ham_octree_iterate_fn fn, void *user){
				return ham_octree_iterate(m_ptr, fn, user);
			}

			template<typename Fn>
			usize iterate(Fn &&fn) noexcept(noexcept(std::forward<Fn>(fn)(std::declval<layer_type>()))){
				struct fn_wrapper{
					Fn *fn;
				} wrapped(&fn);

				constexpr auto dispatcher = +[](ham_octree_layer *layer, void *user) -> bool{
					const auto wrapped = reinterpret_cast<fn_wrapper*>(user);
					if constexpr(std::is_same_v<std::invoke_result_t<Fn, layer_type>, void>){
						(*wrapped->fn)(layer_type(layer));
						return true;
					}
					else{
						return (*wrapped->fn)(layer_type(layer));
					}
				};

				return ham_octree_iterate(m_ptr, dispatcher, &wrapped);
			}

		private:
			ham_octree *m_ptr;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_OCTREE_H
