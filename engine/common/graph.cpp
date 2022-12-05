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
#include "ham/json.h"
#include "ham/functional.h"

#include <cinttypes>

using namespace ham::typedefs;

HAM_C_API_BEGIN

struct ham_graph{
	const ham_allocator *allocator;
	const ham_typeset *ts;
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

ham_graph *ham_graph_create(ham_str8 name, const ham_typeset *ts){
	if(
		!ham_check((name.len && name.ptr) || (!name.len && !name.ptr)) ||
		!ham_check(name.len < (HAM_NAME_BUFFER_SIZE - 1)) ||
		!ham_check(ts != NULL)
	){
		return nullptr;
	}

	const auto allocator = ham_current_allocator();

	const auto graph = ham_allocator_new(allocator, ham_graph);

	graph->allocator = allocator;
	graph->ts = ts;

	if(name.len){
		memcpy(graph->name, name.ptr, name.len);
	}

	graph->name[name.len] = '\0';

	return graph;
}

ham_nothrow void ham_graph_destroy(ham_graph *graph){
	if(ham_unlikely(!graph)) return;

	const auto allocator = graph->allocator;
	ham_allocator_delete(allocator, graph);
}

ham_nothrow ham_str8 ham_graph_name(const ham_graph *graph){
	if(!ham_check(graph != NULL)) return HAM_EMPTY_STR8;
	return ham::str8((const char*)graph->name);
}

ham_nothrow const ham_typeset *ham_graph_ts(const ham_graph *graph){
	if(!ham_check(graph != NULL)) return nullptr;
	return graph->ts;
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

ham_usize ham_graph_serialize(ham_serialize_kind kind, const ham_graph *graph, ham_write_fn write_fn, void *user){
	if(!ham_check(kind < HAM_SERIALIZE_KIND_COUNT) || !ham_check(graph != NULL) || !ham_check(write_fn != NULL)){
		return (usize)-1;
	}

	if(kind != HAM_SERIALIZE_JSON){
		ham_logapierrorf("Only JSON serialization currently implemented");
		return (usize)-1;
	}

	ham::str_buffer8 buf;

	buf.append("{\"ham_graph\":{");

	const auto num_nodes = ham_graph_num_nodes(graph);
	const auto nodes = ham_graph_const_nodes(graph);

	buf.append("\"name\":\"");
	buf.append(ham_graph_name(graph));
	buf.append("\",\"nodes\":[");

	for(usize i = 0; i < num_nodes; i++){
		const auto node = nodes[i];

		const ham::str8 node_name = ham_graph_node_name(node);

		buf.append("{\"name\":\"");
		buf.append(node_name);
		buf.append("\",\"pins\":[");

		const auto num_pins = ham_graph_node_num_pins(node);
		const auto pins = ham_graph_node_const_pins(node);

		for(usize j = 0; j < num_pins; j++){
			const auto pin = pins[j];

			const auto pin_name = ham_graph_node_pin_name(pin);
			const auto pin_type = ham_graph_node_pin_type(pin);
			const auto pin_type_str = ham_type_str(pin_type);
			const auto pin_dir = ham_graph_node_pin_get_direction(pin);

			buf.append("{\"name\":\"");
			buf.append(pin_name);
			buf.append("\",\"type\":\"");
			buf.append(pin_type_str);
			buf.append("\",\"dir\":");
			buf.append(pin_dir == HAM_GRAPH_NODE_PIN_IN ? "0}" : "1}");

			if(j < (num_pins - 1)){
				buf.append(",");
			}
		}

		buf.append("]}");

		if(i < (num_nodes - 1)){
			buf.append(",");
		}
	}

	buf.append("]}}");

	return write_fn(buf.len(), buf.c_str(), user);
}

ham_graph *ham_graph_deserialize_json(const ham_typeset *ts, const ham_json_value *json){
	if(!ham_check(ts != NULL)){
		return nullptr;
	}

	const auto graph = ham_graph_create(HAM_EMPTY_STR8, ts);
	if(!graph){
		ham_logapierrorf("Failed to create graph");
		return nullptr;
	}

	if(!json){
		return graph;
	}

	ham::scope_exit graph_destroyer([graph]{ ham_graph_destroy(graph); });

	static const auto apiname = ham_funcname;

#define json_errorf(fmt_str, ...) ham::logerror(apiname, "Invalid graph json: " fmt_str __VA_OPT__(,) __VA_ARGS__);

	const ham::json_value_view root = json;

	if(!root.is_object()){
		json_errorf("expected root object");
		return nullptr;
	}

	const auto graph_root = root["ham_graph"];
	if(!graph_root){
		json_errorf("no 'ham_graph' object key found");
		return nullptr;
	}

	if(!graph_root.object_validate({ {"name", ham::json_type::string}, {"nodes", ham::json_type::array} })){
		json_errorf("invalid 'ham_graph' object");
		return nullptr;
	}

	const auto graph_name = graph_root["name"].get_str();

	ham_graph_set_name(graph, graph_name);

	const auto nodes = graph_root["nodes"];
	if(!nodes.is_array()){
		json_errorf("invalid 'ham_graph.nodes' value, expected array type");
		return nullptr;
	}

	const usize good_nodes = nodes.array_iterate(
		[&](usize idx, ham::json_value_view<> node){
			if(
				!node.object_validate({
					{"name", ham::json_type::string},
					{"pins", ham::json_type::array}
				}))
			{
				json_errorf("invalid node at element {} of 'nodes'", idx);
				return false;
			}

			const auto name = node["name"].get_str();

			const auto node_ptr = ham_graph_node_create(graph, name);
			if(!node_ptr){
				json_errorf("failed to create graph node '{}'", name);
				return false;
			}

			const auto pins = node["pins"];

			const usize good_pins = pins.array_iterate(
				[&](usize pin_idx, ham::json_value_view<> pin){
					if(!pin.object_validate({ {"name", ham::json_type::string}, {"type", ham::json_type::string}, {"dir", ham::json_type::nat} })){
						json_errorf("invalid pin at element {} of node {}", pin_idx, idx);
						return false;
					}

					const auto pin_type_str = pin["type"].get_str();

					const auto pin_type = ham_typeset_get(ts, pin_type_str);
					if(!pin_type){
						json_errorf("invalid pin type '{}'", pin_type_str);
						return false;
					}

					const auto pin_dir = pin["dir"].get_nat();
					const auto pin_name = pin["name"].get_str();

					const auto pin_ptr = ham_graph_node_pin_create(
						node_ptr,
						pin_dir ? HAM_GRAPH_NODE_PIN_OUT : HAM_GRAPH_NODE_PIN_IN,
						pin_name,
						pin_type
					);

					if(!pin_ptr){
						ham::logerror(apiname, "Could not create graph node pin '{}' for node '{}'", pin_name, name);
						return false;
					}

					return true;
				}
			);

			if(good_pins != pins.array_len()){
				return false;
			}

			return true;
		}
	);

	if(good_nodes != nodes.array_len()){
		return nullptr;
	}

	graph_destroyer.release();
	return graph;
}

ham_graph *ham_graph_deserialize(ham_deserialize_kind kind, const ham_typeset *ts, ham_usize len, const void *data){
	if(!ham_check(kind < HAM_DESERIALIZE_KIND_COUNT) || !ham_check((len && data) || (!len && !data))){
		return nullptr;
	}

	if(!len){
		return ham_graph_create(HAM_EMPTY_STR8, ts);
	}

	switch(kind){
		case HAM_DESERIALIZE_JSON:{
			const auto json_doc = ham_json_document_create({ (const ham_char8*)data, len });
			if(!json_doc){
				ham_logapierrorf("Failed to load graph json");
				return nullptr;
			}

			const auto ret = ham_graph_deserialize_json(ts, ham_json_document_root(json_doc));
			if(!ret){
				ham_logapierrorf("Failed to parse graph json");
			}

			ham_json_document_destroy(json_doc);
			return ret;
		}

		case HAM_DESERIALIZE_BINARY:{
			ham_logapierrorf("Binary graph serialization currently unimplemented");
			return nullptr;
		}

		default:{
			ham_logapierrorf("Unrecognized ham_deserialize_kind value for 'kind': 0x%x", kind);
			return nullptr;
		}
	}
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
