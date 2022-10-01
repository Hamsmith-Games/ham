#include "ham/json.h"

#include "ham/check.h"
#include "ham/memory.h"
#include "ham/fs.h"

#include "yyjson.h"

HAM_C_API_BEGIN

struct ham_json_document{
	const ham_allocator *allocator;
	yyjson_doc *yydoc;
	yyjson_val *yyroot;
};

// NOTE: ham_json_value is just a strong type alias for yyjson_val

ham_json_document *ham_json_document_create(ham_str8 json){
	const auto allocator = ham_current_allocator();

	const auto ptr = ham_allocator_new(allocator, ham_json_document);
	if(!ptr) return nullptr;

	ptr->allocator = allocator;

	ptr->yydoc = yyjson_read(json.ptr, json.len, 0);
	if(!ptr->yydoc){
		ham_logapierrorf("Error in yyjson_read");
		ham_allocator_delete(allocator, ptr);
		return nullptr;
	}

	ptr->yyroot = yyjson_doc_get_root(ptr->yydoc);
	if(!ptr->yyroot){
		ham_logapierrorf("Error in yyjson_doc_get_root");
		ham_allocator_delete(allocator, ptr);
		return nullptr;
	}

	return ptr;
}

ham_json_document *ham_json_document_open(ham_str8 path){
	const auto file = ham_file_open_utf8(path, HAM_OPEN_READ);
	if(!file){
		ham_logapierrorf("Error opening file: %.*s", (int)path.len, path.ptr);
		return nullptr;
	}

	ham_file_info file_info;
	if(!ham_file_get_info(file, &file_info)){
		ham_logapierrorf("Failed to get file info: %.*s", (int)path.len, path.ptr);
		ham_file_close(file);
		return nullptr;
	}

	const auto mapping = ham_file_map(file, HAM_OPEN_READ, 0, file_info.size);
	if(!mapping){
		ham_logapierrorf("Failed to map file contents: %.*s", (int)path.len, path.ptr);
		ham_file_close(file);
		return nullptr;
	}

	const auto ret = ham_json_document_create(ham_str8{ (const ham_char8*)mapping, file_info.size-1 });
	ham_file_unmap(file, mapping, file_info.size);
	ham_file_close(file);

	if(!ret){
		ham_logapierrorf("Failed to create JSON document from file: %.*s", (int)path.len, path.ptr);
	}

	return ret;
}

void ham_json_document_destroy(ham_json_document *doc){
	if(ham_unlikely(doc == NULL)) return;

	const auto allocator = doc->allocator;

	yyjson_doc_free(doc->yydoc);

	ham_allocator_delete(allocator, doc);
}

//
// Generic functions for all values
//

ham_json_type ham_json_get_type(const ham_json_value *json){
	if(!ham_check(json != NULL)) return HAM_JSON_TYPE_COUNT;

	const auto yyval = (yyjson_val*)json;

	switch(unsafe_yyjson_get_type(yyval)){
		case YYJSON_TYPE_NULL: return HAM_JSON_NULL;
		case YYJSON_TYPE_OBJ: return HAM_JSON_OBJECT;
		case YYJSON_TYPE_ARR: return HAM_JSON_ARRAY;
		case YYJSON_TYPE_BOOL: return HAM_JSON_BOOL;

		case YYJSON_TYPE_NUM:{
			switch(unsafe_yyjson_get_subtype(yyval)){
				case YYJSON_SUBTYPE_UINT: return HAM_JSON_NAT;
				case YYJSON_SUBTYPE_SINT: return HAM_JSON_INT;

				case YYJSON_SUBTYPE_REAL:
				default: return HAM_JSON_REAL;
			}
		}

		case YYJSON_TYPE_STR: return HAM_JSON_STRING;
		case YYJSON_TYPE_RAW: return HAM_JSON_RAW;

		default: return HAM_JSON_TYPE_COUNT;
	}
}

//
// Type introspection
//

bool ham_json_is_object(const ham_json_value *json){ return ham_check(json != NULL) ? unsafe_yyjson_is_obj((yyjson_val*)json) : false; }
bool ham_json_is_array(const ham_json_value *json){ return ham_check(json != NULL) ? unsafe_yyjson_is_arr((yyjson_val*)json) : false; }
bool ham_json_is_bool(const ham_json_value *json){ return ham_check(json != NULL) ? unsafe_yyjson_is_bool((yyjson_val*)json) : false; }
bool ham_json_is_nat(const ham_json_value *json){ return ham_check(json != NULL) ? unsafe_yyjson_is_uint((yyjson_val*)json) : false; }
bool ham_json_is_int(const ham_json_value *json){ return ham_check(json != NULL) ? unsafe_yyjson_is_int((yyjson_val*)json) : false; }
bool ham_json_is_real(const ham_json_value *json){ return ham_check(json != NULL) ? unsafe_yyjson_is_real((yyjson_val*)json) : false; }
bool ham_json_is_str(const ham_json_value *json){ return ham_check(json != NULL) ? unsafe_yyjson_is_str((yyjson_val*)json) : false; }
bool ham_json_is_raw(const ham_json_value *json){ return ham_check(json != NULL) ? unsafe_yyjson_is_raw((yyjson_val*)json) : false; }

//
// Object values
//

const ham_json_value *ham_json_document_root(const ham_json_document *doc){
	if(!ham_check(doc != NULL)) return nullptr;

	return (const ham_json_value*)doc->yyroot;
}

const ham_json_value *ham_json_object_get(const ham_json_value *json, const char *key){
	if(!ham_check(json != NULL) || !ham_check(key != NULL)){
		return nullptr;
	}

	// not doing this again.
	const auto ret = yyjson_obj_get(reinterpret_cast<yyjson_val*>(const_cast<ham_json_value*>(json)), key);
	return (const ham_json_value*)ret;
}

ham_usize ham_json_object_iterate(const ham_json_value *json, ham_json_object_iterate_fn fn, void *user){
	if(!ham_check(json != NULL)) return (ham_usize)-1;

	const auto yyval = (yyjson_val*)json;

	if(!yyjson_is_obj(yyval)){
		ham_logapierrorf("JSON value is not an object");
		return (ham_usize)-1;
	}

	if(fn){
		yyjson_obj_iter obj_iter;
		yyjson_obj_iter_init(yyval, &obj_iter);

		yyjson_val *key, *value;

		ham_usize idx = 0;

		while((key = yyjson_obj_iter_next(&obj_iter))){
			value = yyjson_obj_iter_get_val(key);

			const auto key_ptr = unsafe_yyjson_get_str(key);
			const auto key_len = unsafe_yyjson_get_len(key);

			if(!fn((ham_str8){ key_ptr, key_len }, (const ham_json_value*)value, user)){
				return idx;
			}

			++idx;
		}

		return idx;
	}
	else{
		return unsafe_yyjson_get_len(yyval);
	}
}

//
// Array values
//

const ham_json_value *ham_json_array_get(const ham_json_value *json, ham_usize idx){
	if(!ham_check(json != NULL) || !ham_check(idx != (ham_usize)-1)) return nullptr;

	const auto yyval = (yyjson_val*)json;

	if(!yyjson_is_arr(yyval)){
		ham_logapierrorf("JSON value is not an array");
		return nullptr;
	}

	return (const ham_json_value*)yyjson_arr_get(yyval, idx);
}

ham_usize ham_json_array_iterate(const ham_json_value *json, ham_json_array_iterate_fn fn, void *user){
	if(!ham_check(json != NULL)) return (ham_usize)-1;

	const auto yyval = (yyjson_val*)json;

	if(!yyjson_is_arr(yyval)){
		ham_logapierrorf("JSON value is not an array");
		return (ham_usize)-1;
	}

	if(fn){
		ham_usize idx, max;
		yyjson_val *value;
		yyjson_arr_foreach(yyval, idx, max, value){
			if(!fn(idx, (const ham_json_value*)value, user)) return idx;
		}
	}

	return unsafe_yyjson_get_len(yyval);
}

//
// Numeric values
//

ham_uptr ham_json_get_nat(const ham_json_value *json){
	if(!ham_check(json != NULL)) return (ham_uptr)-1;

	const auto yyval = (yyjson_val*)json;

	if(!yyjson_is_uint(yyval)){
		ham_logapierrorf("JSON value is not a natural number");
		return (ham_uptr)-1;
	}

	return unsafe_yyjson_get_uint((yyjson_val*)json);
}

ham_iptr ham_json_get_int(const ham_json_value *json){
	if(!ham_check(json != NULL)) return 0;

	const auto yyval = (yyjson_val*)json;

	if(!yyjson_is_sint(yyval)){
		ham_logapierrorf("JSON value is not an integer");
		return 0;
	}

	return unsafe_yyjson_get_sint((yyjson_val*)json);
}

ham_f64 ham_json_get_real(const ham_json_value *json){
	if(!ham_check(json != NULL)) return 0;

	const auto yyval = (yyjson_val*)json;

	if(!yyjson_is_real(yyval)){
		ham_logapierrorf("JSON value is not a real number");
		return 0.0;
	}

	return unsafe_yyjson_get_real(yyval);
}

//
// Strings
//

ham_str8 ham_json_get_str(const ham_json_value *json){
	if(!ham_check(json != NULL)) return HAM_EMPTY_STR8;

	const auto yyval = (yyjson_val*)json;

	if(!yyjson_is_str(yyval)){
		ham_logapierrorf("JSON value is not a string");
		return HAM_EMPTY_STR8;
	}

	const auto ptr = unsafe_yyjson_get_str(yyval);
	if(!ptr) return HAM_EMPTY_STR8;

	const auto len = unsafe_yyjson_get_len(yyval);
	if(!len) return HAM_EMPTY_STR8;

	return (ham_str8){ ptr, len };
}

HAM_C_API_END
