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

using namespace ham::typedefs;

HAM_C_API_BEGIN

struct ham_graph{
	const ham_allocator *allocator;
	ham_name_buffer_utf8 name;
	ham::basic_buffer<ham_graph_node*> nodes;
};

struct ham_graph_node{
	ham_graph *graph;
	ham_name_buffer_utf8 name;
	ham::basic_buffer<ham_graph_node_pin*> pins;
};

struct ham_graph_node_pin{
	ham_graph_node *node;
	ham_name_buffer_utf8 name;
	ham::type type;
	ham_graph_node_pin_kind kind;
	ham_graph_node_connection conn;
};

//
// Special graph exec type
//

constexpr ham::str8 ham_impl_graph_exec_type_name = "_ham_graph_exec";

ham_nonnull_args(1)
static inline const ham_type *ham_impl_build_graph_exec_type(ham_typeset *ts){
	ham::type_builder builder;

	builder.set_kind(HAM_TYPE_OBJECT);
	builder.set_name(ham_impl_graph_exec_type_name);

	return builder.instantiate(ts);
}

const ham_type *ham_graph_exec_type(ham_typeset *ts){
	auto ret = ham_typeset_get(ts, ham_impl_graph_exec_type_name);
	if(!ret) ret = ham_impl_build_graph_exec_type(ts);
	return ret;
}

//
// Graph
//

ham_graph *ham_graph_create(ham_str8 name){
	if(!ham_check(name.len && name.ptr) || !ham_check(name.len < (HAM_NAME_BUFFER_SIZE - 1))) return nullptr;

	const auto allocator = ham_current_allocator();

	const auto graph = ham_allocator_new(allocator, ham_graph);

	graph->allocator = allocator;

	memcpy(graph->name, name.ptr, name.len);
	graph->name[name.len] = '\0';

	return graph;
}

void ham_graph_destroy(ham_graph *graph){
	if(ham_unlikely(!graph)) return;

	const auto allocator = graph->allocator;
	ham_allocator_delete(allocator, graph);
}

ham_str8 ham_graph_name(const ham_graph *graph){
	if(!ham_check(graph != NULL)) return HAM_EMPTY_STR8;
	return ham::str8((const char*)graph->name);
}

bool ham_graph_set_name(ham_graph *graph, ham_str8 new_name){
	if(
		!ham_check(graph != NULL) ||
		!ham_check(new_name.len && new_name.ptr) ||
		!ham_check(new_name.len < (HAM_NAME_BUFFER_SIZE - 1))
	){
		return false;
	}

	memcpy(graph->name, new_name.ptr, new_name.len);
	graph->name[new_name.len] = '\0';

	return true;
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
	   !ham_check(name.len && name.ptr) ||
	   !ham_check(name.len < (HAM_NAME_BUFFER_SIZE - 1))
	){
		return nullptr;
	}

	const auto allocator = graph->allocator;

	const auto node = ham_allocator_new(allocator, ham_graph_node);

	node->graph = graph;

	memcpy(node->name, name.ptr, name.len);
	node->name[name.len] = '\0';

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

ham_nothrow ham_graph *ham_graph_node_graph(const ham_graph_node *node){
	if(!ham_check(node != NULL)) return nullptr;
	return node->graph;
}

ham_nothrow ham_str8 ham_graph_node_name(const ham_graph_node *node){
	if(!ham_check(node != NULL)) return HAM_EMPTY_STR8;
	return ham::str8((const char*)node->name);
}

ham_nothrow bool ham_graph_node_set_name(ham_graph_node *node, ham_str8 new_name){
	if(!ham_check(node != NULL) || !ham_check(new_name.len && new_name.ptr) || !ham_check(new_name.len < (HAM_NAME_BUFFER_SIZE - 1))){
		return false;
	}

	memcpy(node->name, new_name.ptr, new_name.len);
	node->name[new_name.len] = '\0';

	return true;
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

ham_graph_node_pin *ham_graph_node_pin_create(
	ham_graph_node *node,
	ham_graph_node_pin_direction direction,
	ham_str8 name,
	const ham_type *type
){
	if(
	   !ham_check(node != NULL) ||
	   !ham_check(direction < HAM_GRAPH_NODE_PIN_DIRECTION_COUNT) ||
	   !ham_check(name.len && name.ptr) ||
	   !ham_check(name.len < (HAM_NAME_BUFFER_SIZE - 1)) ||
	   !ham_check(type != NULL)
	){
		return nullptr;
	}

	const auto allocator = node->graph->allocator;

	const auto pin = ham_allocator_new(allocator, ham_graph_node_pin);

	pin->node = node;

	memcpy(pin->name, name.ptr, name.len);
	pin->name[name.len] = '\0';

	pin->type = type;

	if(direction == HAM_GRAPH_NODE_PIN_IN){
		pin->conn.in  = pin;
		pin->conn.out = nullptr;
	}
	else{
		pin->conn.in  = nullptr;
		pin->conn.out = pin;
	}

	if(ham_impl_graph_exec_type_name == ham::str8(ham_type_name(type))){
		pin->kind = (ham_graph_node_pin_kind)(HAM_GRAPH_NODE_PIN_EXEC_IN + (int)direction);
	}
	else{
		pin->kind = (ham_graph_node_pin_kind)(HAM_GRAPH_NODE_PIN_DATA_IN + (int)direction);
	}

	return pin;
}

void ham_graph_node_pin_destroy(ham_graph_node_pin *pin){
	if(ham_unlikely(!pin)) return;

	const auto allocator = pin->node->graph->allocator;

	ham_allocator_delete(allocator, pin);
}

bool ham_graph_node_pin_connect(ham_graph_node_pin *pin, ham_graph_node_pin *other){
	if(!ham_check(pin != NULL)) return false;

	if(ham_graph_node_pin_get_direction(pin) == HAM_GRAPH_NODE_PIN_OUT){
		if(other){
			if(!ham_check(ham_graph_node_pin_get_direction(other) == HAM_GRAPH_NODE_PIN_IN)){
				return false;
			}

			other->conn.out = pin;
		}

		pin->conn.in = other;
	}
	else{ // input pin
		if(other){
			if(!ham_check(ham_graph_node_pin_get_direction(other) == HAM_GRAPH_NODE_PIN_OUT)){
				return false;
			}

			other->conn.in = pin;
		}

		pin->conn.out = other;
	}

	return true;
}

ham_nothrow ham_str8 ham_graph_node_pin_name(const ham_graph_node_pin *pin){
	if(!ham_check(pin != NULL)) return HAM_EMPTY_STR8;
	return ham::str8((const char*)pin->name);
}

ham_nothrow const ham_type *ham_graph_node_pin_type(const ham_graph_node_pin *pin){
	if(!ham_check(pin != NULL)) return nullptr;
	return pin->type;
}

ham_nothrow bool ham_graph_node_pin_set_name(ham_graph_node_pin *pin, ham_str8 new_name){
	if(
	   !ham_check(pin != NULL) ||
	   !ham_check(new_name.len && new_name.ptr) ||
	   !ham_check(new_name.len < (HAM_NAME_BUFFER_SIZE - 1))
	){
		return false;
	}

	memcpy(pin->name, new_name.ptr, new_name.len);
	pin->name[new_name.len] = '\0';

	return true;
}

ham_nothrow bool ham_graph_node_pin_set_type(ham_graph_node_pin *pin, const ham_type *new_type){
	if(!ham_check(pin != NULL) || !ham_check(new_type != NULL)) return false;

	pin->type = new_type;

	int direction = pin->kind % 2;

	if(ham_impl_graph_exec_type_name == ham_type_name(new_type)){
		pin->kind = (ham_graph_node_pin_kind)(HAM_GRAPH_NODE_PIN_EXEC_IN + direction);
	}
	else{
		pin->kind = (ham_graph_node_pin_kind)(HAM_GRAPH_NODE_PIN_DATA_IN + direction);
	}

	return true;
}

ham_nothrow ham_graph_node *ham_graph_node_pin_node(const ham_graph_node_pin *pin){
	if(!ham_check(pin != NULL)) return nullptr;
	return pin->node;
}

ham_nothrow ham_graph_node_pin_kind ham_graph_node_pin_get_kind(const ham_graph_node_pin *pin){
	return pin->kind;
}

ham_nothrow bool ham_graph_node_pin_is_connected(const ham_graph_node_pin *pin){
	if(ham_graph_node_pin_get_direction(pin) == HAM_GRAPH_NODE_PIN_OUT){
		return pin->conn.in != nullptr;
	}
	else{
		return pin->conn.out != nullptr;
	}
}

ham_nothrow const ham_graph_node_connection *ham_graph_node_pin_connection(const ham_graph_node_pin *pin){
	return ham_graph_node_pin_is_connected(pin) ? &pin->conn : nullptr;
}

HAM_C_API_END
