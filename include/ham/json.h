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

#ifndef HAM_JSON_H
#define HAM_JSON_H 1

/**
 * @defgroup HAM_JSON JSON Parsing
 * @ingroup HAM
 * @{
 */

#include "ham/typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_json_document ham_json_document;

typedef struct ham_json_value ham_json_value;

ham_api ham_json_document *ham_json_document_create(ham_str8 json);

ham_api ham_json_document *ham_json_document_open(ham_str8 path);

ham_api void ham_json_document_destroy(ham_json_document *doc);

ham_api const ham_json_value *ham_json_document_root(const ham_json_document *doc);

typedef enum ham_json_type{
	HAM_JSON_NULL,
	HAM_JSON_OBJECT,
	HAM_JSON_ARRAY,
	HAM_JSON_BOOL,
	HAM_JSON_NAT,
	HAM_JSON_INT,
	HAM_JSON_REAL,
	HAM_JSON_STRING,
	HAM_JSON_RAW,

	HAM_JSON_TYPE_COUNT
} ham_json_type;

ham_api ham_json_type ham_json_get_type(const ham_json_value *json);

ham_api bool ham_json_is_null(const ham_json_value *json);
ham_api bool ham_json_is_object(const ham_json_value *json);
ham_api bool ham_json_is_array(const ham_json_value *json);
ham_api bool ham_json_is_bool(const ham_json_value *json);
ham_api bool ham_json_is_nat(const ham_json_value *json);
ham_api bool ham_json_is_int(const ham_json_value *json);
ham_api bool ham_json_is_real(const ham_json_value *json);
ham_api bool ham_json_is_str(const ham_json_value *json);
ham_api bool ham_json_is_raw(const ham_json_value *json);

// Objects

ham_api const ham_json_value *ham_json_object_get(const ham_json_value *json, const char *key);

typedef bool(*ham_json_object_iterate_fn)(ham_str8 key, const ham_json_value *value, void *user);

ham_api ham_usize ham_json_object_iterate(const ham_json_value *json, ham_json_object_iterate_fn fn, void *user);

// Arrays

ham_api const ham_json_value *ham_json_array_get(const ham_json_value *json, ham_usize idx);

typedef bool(*ham_json_array_iterate_fn)(ham_usize idx, const ham_json_value *value, void *user);

ham_api ham_usize ham_json_array_iterate(const ham_json_value *json, ham_json_array_iterate_fn fn, void *user);

// Numeric

ham_api ham_uptr ham_json_get_nat(const ham_json_value *json);

ham_api ham_iptr ham_json_get_int(const ham_json_value *json);

ham_api ham_f64 ham_json_get_real(const ham_json_value *json);

// Strings

ham_api ham_str8 ham_json_get_str(const ham_json_value *json);

HAM_C_API_END

#ifdef __cplusplus

#include "str_buffer.h"

namespace ham{
	enum class json_type{
		null = HAM_JSON_NULL,
		object = HAM_JSON_OBJECT,
		array = HAM_JSON_ARRAY,
		bool_ = HAM_JSON_BOOL,
		nat = HAM_JSON_NAT,
		int_ = HAM_JSON_INT,
		real = HAM_JSON_REAL,
		string = HAM_JSON_STRING,
		raw = HAM_JSON_RAW,
	};

	template<bool Mutable = false>
	class json_value_view{
		public:
			using handle_type = std::conditional_t<Mutable, ham_json_value*, const ham_json_value*>;

			json_value_view(handle_type val = nullptr) noexcept
				: m_val(val){}

			operator bool() const noexcept{ return m_val != nullptr; }

			template<bool UMutable = Mutable>
			operator handle_type() const noexcept{ return m_val; }

			json_type type() const noexcept{
				return static_cast<json_type>(ham_json_get_type(m_val));
			}

			usize object_len() const noexcept{ return ham_json_object_iterate(m_val, nullptr, nullptr); }
			json_value_view<Mutable> object_get(const char *key) const noexcept{ return ham_json_object_get(m_val, key); }

			template<typename Fn>
			usize object_iterate(Fn &&f) const noexcept(noexcept(std::forward<Fn>(f)(std::declval<const ham_json_value*>()))){
				constexpr auto wrapper = +[](const ham_json_value *val, void *user) -> bool{
					const auto fptr = reinterpret_cast<std::remove_reference_t<Fn>*>(user);
					return (*fptr)(val);
				};

				return ham_json_object_iterate(m_val, wrapper, &f);
			}

			usize array_len() const noexcept{ return ham_json_array_iterate(m_val, nullptr, nullptr); }
			json_value_view<Mutable> array_get(usize idx) const noexcept{ return ham_json_array_get(m_val, idx); }

			template<typename Fn>
			usize array_iterate(Fn &&f) const noexcept(noexcept(std::forward<Fn>(f)(std::declval<const ham_json_value*>()))){
				constexpr auto wrapper = +[](const ham_json_value *val, void *user) -> bool{
					const auto fptr = reinterpret_cast<std::remove_reference_t<Fn>*>(user);
					return (*fptr)(val);
				};

				return ham_json_array_iterate(m_val, wrapper, &f);
			}

			json_value_view<Mutable> operator[](usize idx) const noexcept{
				const auto obj_type = type();
				switch(obj_type){
					case json_type::array: return array_get(idx);

					case json_type::object:{
						char num_buf[21] = { 0 }; // 2^64=18446744073709551616 is 20 sf
						format_buffered(sizeof(num_buf), num_buf, "{}", idx);
						return object_get(num_buf);
					}

					default: return nullptr;
				}
			}

			json_value_view<Mutable> operator[](const char *key) const noexcept{
				switch(type()){
					case json_type::object: return object_get(key);
					default: return nullptr;
				}
			}

			bool is_null()   const noexcept{ return ham_json_is_null(m_val); }
			bool is_object() const noexcept{ return ham_json_is_object(m_val); }
			bool is_array()  const noexcept{ return ham_json_is_array(m_val); }
			bool is_bool()   const noexcept{ return ham_json_is_bool(m_val); }
			bool is_nat()    const noexcept{ return ham_json_is_nat(m_val); }
			bool is_int()    const noexcept{ return ham_json_is_int(m_val); }
			bool is_real()   const noexcept{ return ham_json_is_real(m_val); }
			bool is_str()    const noexcept{ return ham_json_is_str(m_val); }
			bool is_raw()    const noexcept{ return ham_json_is_raw(m_val); }

			uptr get_nat()  const noexcept{ return ham_json_get_nat(m_val); }
			iptr get_int()  const noexcept{ return ham_json_get_int(m_val); }
			f64  get_real() const noexcept{ return ham_json_get_real(m_val); }
			str8 get_str()  const noexcept{ return ham_json_get_str(m_val); }

		private:
			handle_type m_val;
	};

	json_value_view(ham_json_value*) -> json_value_view<true>;
	json_value_view(const ham_json_value*) -> json_value_view<false>;

	class json_document{
		public:
			json_document(const str8 &json)
				: m_handle(ham_json_document_create(json)){}

			static json_document open(const str8 &path){
				return json_document(path_tag{}, path);
			}

			operator bool() const& noexcept{ return m_handle.get() != nullptr; }

			json_value_view<false> root() const noexcept{ return ham_json_document_root(m_handle.get()); }

		private:
			struct path_tag{};

			json_document(path_tag, const str8 &path)
				: m_handle(ham_json_document_open(path)){}

			unique_handle<ham_json_document*, ham_json_document_destroy> m_handle;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_JSON_H
