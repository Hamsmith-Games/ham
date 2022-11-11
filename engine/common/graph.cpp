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

#include "ham/engine/graph.h"

#include "ham/check.h"
#include "ham/buffer.h"
#include "ham/str_buffer.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

struct ham_graph{
	const ham_allocator *allocator;
	ham::str_buffer_utf8 name;
	ham::basic_buffer<ham_graph_node*> nodes;
};

struct ham_graph_node{
	ham_graph *graph;
	ham::str_buffer_utf8 name;
	ham::basic_buffer<ham_graph_node_pin*> pins;
};

struct ham_graph_node_pin{
	ham_graph_node *node;
	ham::str_buffer_utf8 name;
	ham::type type;
};

struct ham_graph_node_connection{
	const ham_allocator *allocator;
	ham_graph_node_pin *in, *out;
};

//
// Graph
//

ham_graph *ham_graph_create(ham_str8 name){
	if(!ham_check(name.len && name.ptr)) return nullptr;

	const auto allocator = ham_current_allocator();

	const auto graph = ham_allocator_new(allocator, ham_graph);

	graph->allocator = allocator;
	graph->name      = name;

	return graph;
}

void ham_graph_destroy(ham_graph *graph){
	if(ham_unlikely(!graph)) return;

	const auto allocator = graph->allocator;
	ham_allocator_delete(allocator, graph);
}

ham_str8 ham_graph_name(const ham_graph *graph){
	if(!ham_check(graph != NULL)) return HAM_EMPTY_STR8;
	return graph->name;
}

ham_usize ham_graph_num_nodes(const ham_graph *graph){
	if(!ham_check(graph != NULL)) return (ham_usize)-1;
	return graph->nodes.size();
}

ham_graph_node *const *ham_graph_nodes(ham_graph *graph){
	if(!ham_check(graph != NULL)) return nullptr;
	return graph->nodes.data();
}

const ham_graph_node *const *ham_graph_const_nodes(const ham_graph *graph){
	if(!ham_check(graph != NULL)) return nullptr;
	return graph->nodes.data();
}

//
// Nodes
//

ham_graph_node *ham_graph_node_create(ham_graph *graph, ham_str8 name){
	if(
	   !ham_check(graph != NULL) ||
	   !ham_check(name.len && name.ptr)
	){
		return nullptr;
	}

	const auto allocator = graph->allocator;

	const auto node = ham_allocator_new(allocator, ham_graph_node);

	node->graph = graph;
	node->name = name;

	return node;
}

void ham_graph_node_destroy(ham_graph_node *node){
	if(ham_unlikely(!node)) return;

	const auto allocator = node->graph->allocator;

	for(auto pin : node->pins){
		ham_allocator_delete(allocator, pin);
	}

	ham_allocator_delete(allocator, node);
}

ham_usize ham_graph_node_num_pins(const ham_graph_node *node){
	if(!ham_check(node != NULL)) return (ham_usize)-1;
	return node->pins.size();
}

ham_graph_node_pin *const *ham_graph_node_pins(ham_graph_node *node){
	if(!ham_check(node != NULL)) return nullptr;
	return node->pins.data();
}

const ham_graph_node_pin *const *ham_graph_node_const_pins(const ham_graph_node *node){
	if(!ham_check(node != NULL)) return nullptr;
	return node->pins.data();
}

//
// Node pins
//

ham_graph_node_pin *ham_graph_node_pin_create(ham_graph_node *node, ham_str8 name, const ham_type *type){
	if(
	   !ham_check(node != NULL) ||
	   !ham_check(name.len && name.ptr) ||
	   !ham_check(type != NULL)
	){
		return nullptr;
	}

	const auto allocator = node->graph->allocator;

	const auto pin = ham_allocator_new(allocator, ham_graph_node_pin);

	pin->node = node;
	pin->name = name;
	pin->type = type;

	return pin;
}

void ham_graph_node_pin_destroy(ham_graph_node_pin *pin){
	if(ham_unlikely(!pin)) return;

	const auto allocator = pin->node->graph->allocator;

	ham_allocator_delete(allocator, pin);
}

ham_str8 ham_graph_node_pin_name(const ham_graph_node_pin *pin){
	if(!ham_check(pin != NULL)) return HAM_EMPTY_STR8;
	return pin->name;
}

const ham_type *ham_graph_node_pin_type(const ham_graph_node_pin *pin){
	if(!ham_check(pin != NULL)) return nullptr;
	return pin->type;
}

HAM_C_API_END
