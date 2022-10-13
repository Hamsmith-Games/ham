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

#include "ham/octree.h"
#include "ham/check.h"
#include "ham/colony.h"

#include "ham/std_vector.hpp"

HAM_C_API_BEGIN

constexpr static inline ham_usize ham_octree_layer_size(ham_usize level){
	return 8UL << (3UL * level);
}

struct ham_octree_node{
	union {
		ham_octree *tree;
		ham_octree_node *parent;
	};

	ham_usize depth;
	bool is_leaf;

	union {
		void *value;
		ham_octree_node *children[8];
	};
};

struct ham_octree_layer{
	ham_octree_layer(ham_usize leaf_alignment, ham_usize leaf_size)
		: values(leaf_alignment, leaf_size)
	{}

	ham_octree *tree;
	ham_usize depth, max_size;
	ham::colony<> values;
	ham::colony<ham_octree_node> nodes;
};

static ham_octree_layer *ham_octree_layer_create(ham_octree *tree, ham_usize depth);
ham_nothrow static void ham_octree_layer_destroy(ham_octree_layer *layer);

struct ham_octree{
	const ham_allocator *allocator;
	ham_usize leaf_alignment, leaf_size;
	ham::std_vector<ham::unique_handle<ham_octree_layer*, ham_octree_layer_destroy>> layers;
	ham_octree_node *root;
};

static inline ham_octree *ham_impl_octree_node_tree(ham_octree_node *node){
	while(node->depth > 0) node = node->parent;
	return node->tree;
}

static ham_octree_node *ham_impl_octree_node_emplace(ham_octree_node *parent, ham_usize idx, bool is_leaf){
	const auto layer = ham_octree_layer_at(ham_impl_octree_node_tree(parent), parent->depth + 1);
	if(!layer){
		ham_logapierrorf("Failed to get octree layer at depth %zu", idx);
		return nullptr;
	}

	void *leaf_value = nullptr;

	if(is_leaf){
		leaf_value = layer->values.emplace();
		if(!leaf_value){
			ham_logapierrorf("Failed to allocate leaf value in layer at depth %zu", idx);
			return nullptr;
		}
	}

	const auto node = layer->nodes.emplace();
	if(!node){
		ham_logapierrorf("Failed to emplace node in layer at depth %zu", idx);
		return nullptr;
	}

	node->parent = parent;
	node->depth = parent->depth + 1;
	node->is_leaf = is_leaf;

	if(is_leaf){
		node->value = leaf_value;
	}
	else{
		memset(node->children, 0, sizeof(node->children));
	}

	parent->children[idx] = node;

	return node;
}

ham_octree_layer *ham_octree_layer_create(ham_octree *tree, ham_usize depth){
	if(
	   !ham_check(tree != NULL)
	){
		return nullptr;
	}

	const auto layer = ham_allocator_new(tree->allocator, ham_octree_layer, tree->leaf_alignment, tree->leaf_size);
	if(!layer){
		ham_logapierrorf("Error allocating ham_octree_layer");
		return nullptr;
	}

	layer->tree = tree;
	layer->depth = depth;
	layer->max_size = ham_octree_layer_size(depth) * tree->leaf_size;

	return layer;
}

ham_nothrow void ham_octree_layer_destroy(ham_octree_layer *layer){
	if(ham_unlikely(!layer)) return;

	const auto allocator = layer->tree->allocator;

	ham_allocator_delete(allocator, layer);
}

//
// Manager/context type
//

ham_octree *ham_octree_create(ham_usize leaf_alignment, ham_usize leaf_size){
	if(
	   !ham_check(ham_popcnt64(leaf_alignment) == 1) ||
	   !ham_check(leaf_size % leaf_alignment == 0)
	){
		return nullptr;
	}

	const auto allocator = ham_current_allocator();

	const auto tree = ham_allocator_new(allocator, ham_octree);
	if(!tree){
		ham_logapierrorf("Error allocating ham_octree");
		return nullptr;
	}

	tree->allocator = allocator;
	tree->leaf_alignment = leaf_alignment;
	tree->leaf_size = leaf_size;

	const auto root_layer = ham_octree_layer_create(tree, 0);
	if(!root_layer){
		ham_logapierrorf("Error creating root layer");
		ham_allocator_delete(allocator, tree);
		return nullptr;
	}

	tree->layers.emplace_back(root_layer);

	tree->root = root_layer->nodes.emplace();
	if(!tree->root){
		ham_logapierrorf("Error creating root node");
		ham_allocator_delete(allocator, tree);
		return nullptr;
	}

	tree->root->tree = tree;
	tree->root->depth = 0;
	tree->root->is_leaf = false;

	memset(tree->root->children, 0, sizeof(tree->root->children));

	return tree;
}

ham_nothrow void ham_octree_destroy(ham_octree *tree){
	if(ham_unlikely(!tree)) return;

	const auto allocator = tree->allocator;

	ham_allocator_delete(allocator, tree);
}

ham_usize ham_octree_iterate(ham_octree *tree, ham_octree_iterate_fn fn, void *user){
	if(!ham_check(tree != NULL)) return (ham_usize)-1;

	if(fn){
		for(ham_usize i = 0; i < tree->layers.size(); i++){
			const auto layer = tree->layers[i].get();
			if(!fn(layer, user)) return i;
		}
	}

	return tree->layers.size();
}

ham_nothrow ham_octree_node *ham_octree_root(ham_octree *tree){
	if(!ham_check(tree != NULL)) return nullptr;
	return tree->root;
}

ham_nothrow ham_usize ham_octree_current_depth(ham_octree *tree){
	if(!ham_check(tree != NULL)) return (ham_usize)-1;
	return tree->layers.size();
}

ham_octree_layer *ham_octree_layer_at(ham_octree *tree, ham_usize depth){
	if(!ham_check(tree != NULL) || !ham_check(depth > 0) || !ham_check(depth != (ham_usize)-1)) return nullptr;

	if(tree->layers.size() <= depth){
		for(ham_usize i = tree->layers.size(); i <= depth; i++){
			const auto new_layer = ham_octree_layer_create(tree, i);
			if(!new_layer){
				ham_logapierrorf("Failed to create layer at depth %zu", i);
				return nullptr;
			}

			tree->layers.emplace_back(new_layer);
		}
	}

	return tree->layers[depth].get();
}

//
// Layers
//

ham_usize ham_octree_layer_iterate(ham_octree_layer *layer, ham_octree_layer_iterate_fn fn, void *user){
	if(!ham_check(layer != NULL)) return (ham_usize)-1;

	if(fn){
		struct wrapped_user{
			ham_octree_layer_iterate_fn fn;
			void *user;
		} user_wrapped{fn, user};

		constexpr auto wrap_fn = [](void *elem, void *user) -> bool{
			const auto user_wrapped = reinterpret_cast<wrapped_user*>(user);
			const auto node = reinterpret_cast<ham_octree_node*>(elem);
			return user_wrapped->fn(node, user_wrapped->user);
		};

		return layer->nodes.iterate(wrap_fn, &user_wrapped);
	}
	else{
		return layer->nodes.size();
	}
}

ham_nothrow ham_usize ham_octree_layer_num_nodes(const ham_octree_layer *layer){
	if(!ham_check(layer != NULL)) return (ham_usize)-1;
	else return layer->nodes.size();
}

ham_nothrow ham_octree_node *ham_octree_layer_node_at(ham_octree_layer *layer, ham_usize idx){
	if(!ham_check(layer != NULL) || !ham_check(idx < layer->nodes.size())){
		return nullptr;
	}

	return layer->nodes.at(idx);
}

//
// Nodes
//

ham_nothrow bool ham_octree_node_is_root(const ham_octree_node *node){
	if(!ham_check(node != NULL)) return false;
	else return node->depth == 0;
}

ham_nothrow bool ham_octree_node_is_leaf(const ham_octree_node *node){
	if(!ham_check(node != NULL)) return false;
	else return node->is_leaf;
}

ham_nothrow ham_octree_node *ham_octree_node_parent(ham_octree_node *node){
	if(!ham_check(node != NULL) || !ham_check(node->depth > 0)) return nullptr;
	else return node->parent;
}

ham_nothrow ham_octree *ham_octree_node_tree(ham_octree_node *node){
	if(!ham_check(node != NULL)) return nullptr;

	while(node->depth > 0){
		node = node->parent;
	}

	return node->tree;
}

ham_nothrow void *ham_octree_node_value(ham_octree_node *node){
	if(!ham_check(node != NULL) || !ham_check(node->is_leaf == true)) return nullptr;
	else return node->value;
}

ham_nothrow ham_octree_node *ham_octree_node_child(ham_octree_node *node, ham_usize idx){
	if(
	   !ham_check(node != NULL) ||
	   !ham_check(node->is_leaf == false) ||
	   !ham_check(idx < 8)
	){
		return nullptr;
	}
	else return node->children[idx];
}

ham_octree_node *ham_octree_node_emplace(ham_octree_node *parent, ham_usize idx){
	if(
	   !ham_check(parent != NULL) ||
	   !ham_check(parent->is_leaf == false) ||
	   !ham_check(idx < 8) ||
	   !ham_check(parent->children[idx] == NULL)
	){
		return nullptr;
	}

	return ham_impl_octree_node_emplace(parent, idx, false);
}

ham_octree_node *ham_octree_node_emplace_leaf(ham_octree_node *parent, ham_usize idx){
	if(
	   !ham_check(parent != NULL) ||
	   !ham_check(parent->is_leaf == false) ||
	   !ham_check(idx < 8) ||
	   !ham_check(parent->children[idx] == NULL)
	){
		return nullptr;
	}

	return ham_impl_octree_node_emplace(parent, idx, true);
}

bool ham_octree_node_make_leaf(ham_octree_node *node){
	if(
		!ham_check(node != NULL) ||
		node->is_leaf
	){
		return false;
	}

	for(int i = 0; i < 8; i++){
		if(node->children[i]){
			ham_logapierrorf("Child node %d is not NULL", i);
			return false;
		}
	}

	return true;
}

HAM_C_API_END
