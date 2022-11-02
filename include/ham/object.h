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

#ifndef HAM_OBJECT_H
#define HAM_OBJECT_H 1

/**
 * @defgroup HAM_OBJECT Object-oriented utilities
 * @ingroup HAM
 * @{
 */

#include "dso.h"

#include <stdarg.h>

HAM_C_API_BEGIN

#define HAM_SUPER_NAME _ham_super

/**
 * @brief Derivable object with virtual functions.
 */
typedef struct ham_object ham_object;

/**
 * @brief Virtual function table.
 */
typedef struct ham_object_vtable ham_object_vtable;

/**
 * @brief Helper type for easily creating and destroying objects.
 */
typedef struct ham_object_manager ham_object_manager;

/**
 * @brief Create a new object manager for a specified object type.
 * @param vtable vtable for the object type
 * @returns newly created object manager or ``NULL`` on error
 */
ham_api ham_object_manager *ham_object_manager_create(const ham_object_vtable *vtable);

/**
 * @brief Destroy an object manager.
 * @param manager manager to destroy
 */
ham_api void ham_object_manager_destroy(ham_object_manager *manager);

/**
 * @brief Check if an object is managed by a given manager.
 * @param manager manager to query
 * @param obj object to check for
 * @returns whether \p obj is contained in \p mangaer
 */
ham_api bool ham_object_manager_contains(const ham_object_manager *manager, const ham_object *obj);

typedef bool(*ham_object_manager_iterate_fn)(ham_object *obj, void *user);

/**
 * @brief Iterate through all objects in a manager.
 * @note If ``NULL`` if passed for \p fn then this function returns the number of objects contained in \p manager .
 * @param manager manager to iterate
 * @param fn function to call on each object
 * @param user data passed in each call to \p fn
 * @returns number of times \p fn was called before returning ``false``
 */
ham_api ham_usize ham_object_manager_iterate(ham_object_manager *manager, ham_object_manager_iterate_fn fn, void *user);

/**
 * @brief Create a new object, initializing the allocated object memory before constructing it.
 * @param manager manager to create the new object with
 * @param init_fn initialization function, returns ``false`` to signal an error
 * @param user data passed to \p init_fn
 * @param nargs number of arguments passed
 * @param va list for the arguments
 * @returns newly created object or ``NULL`` on error
 */
ham_api ham_object *ham_object_vnew_init(
	ham_object_manager *manager,
	ham_object_manager_iterate_fn init_fn, void *user,
	ham_usize nargs, va_list va
);

/**
 * @brief Create a new object, passing constructor arguments through a ``va_list``.
 * @param manager manager to create the new object with
 * @param nargs number of arguments passed
 * @param va list for the arguments
 * @returns newly created object or ``NULL`` on error
 */
static inline ham_object *ham_object_vnew(ham_object_manager *manager, ham_usize nargs, va_list va){
	return ham_object_vnew_init(manager, nullptr, nullptr, nargs, va);
}

//! @cond ignore
static inline ham_object *ham_impl_object_new_init(
	ham_object_manager *manager,
	ham_object_manager_iterate_fn init_fn, void *user,
	ham_usize nargs, ...
){
	va_list va;
	va_start(va, nargs);
	ham_object *const ret = ham_object_vnew_init(manager, init_fn, user, nargs, va);
	va_end(va);
	return ret;
}
//! @endcond

/**
 * @brief Create a new object, initializing memory before use.
 * @param manager manager to create the new object with
 * @param init_fn initialization function, returns ``false`` on error
 * @param user data passed to \p init_fn
 * @returns newly created object or ``NULL`` on error
 */
#define ham_object_new_init(manager, init_fn, user, ...) (ham_impl_object_new_init(manager, (init_fn), (user), HAM_NARGS(__VA_ARGS__) __VA_OPT__(,) __VA_ARGS__))

/**
 * @brief Create a new object.
 * @param manager manager to create the new object with
 * @returns newly created object or ``NULL`` on error
 */
#define ham_object_new(manager, ...) (ham_impl_object_new_init(manager, nullptr, nullptr, HAM_NARGS(__VA_ARGS__) __VA_OPT__(,) __VA_ARGS__))

/**
 * @brief Destroy a managed object.
 * @param manager manager \p obj belongs to
 * @param obj object to destroy
 * @returns whether the object was successfully destroyed.
 */
ham_api bool ham_object_delete(ham_object_manager *manager, ham_object *obj);

ham_api const ham_object_vtable *ham_object_dso_vptr(ham_dso_handle dso, const char *obj_type_id);

typedef bool(*ham_dso_iterate_object_vptrs_fn)(const ham_object_vtable *vptr, void *user);

static inline ham_usize ham_dso_iterate_object_vptrs(
	ham_dso_handle handle,
	ham_dso_iterate_object_vptrs_fn fn,
	void *user
);

typedef struct ham_object_info{
	const char *type_id;
	ham_usize alignment, size;
} ham_object_info;

struct ham_object_vtable{
	const ham_object_info *info;

	ham_object*(*ctor)(ham_object *ptr, ham_u32 nargs, va_list va);
	void       (*dtor)(ham_object*);
};

struct ham_object{
	const ham_object_vtable *vptr;
};

#define ham_derive(base) base HAM_SUPER_NAME;

#ifdef __cplusplus
#	define ham_impl_def_ctor(object_type, nargs_name, va_name) \
		static inline object_type *object_type##_ctor_user(object_type *self, ham_u32 nargs_name, va_list va_name); \
		static inline object_type *object_type##_ctor(object_type *self, ham_u32 nargs, va_list va){ \
			auto super_slice = *ham_super(self); \
			const auto ptr = new(self) object_type; \
			*ham_super(ptr) = std::move(super_slice);\
			const auto ret = object_type##_ctor_user(ptr, nargs, va); \
			if(!ret) std::destroy_at(ptr); \
			return ret; \
		} \
		object_type *object_type##_ctor_user(object_type *self, ham_u32 nargs_name, va_list va_name)

#	define ham_impl_def_dtor(object_type) \
		ham_nothrow static inline void object_type##_dtor_user(object_type *self); \
		ham_nothrow static inline void object_type##_dtor(object_type *self){ \
			object_type##_dtor_user(self); \
			std::destroy_at(self); \
		} \
		ham_nothrow void object_type##_dtor_user(object_type *self)
#else
#	define ham_impl_def_ctor(object_type, nargs_name, va_name) \
		static inline object_type *object_type##_ctor(object_type *self, ham_u32 nargs_name, va_list va_name)

#	define ham_impl_def_dtor(object_type) \
		ham_nothrow static inline void object_type##_dtor(object_type *self)
#endif

#define ham_def_ctor(object_type, nargs_name, va_name) \
	ham_impl_def_ctor(object_type, nargs_name, va_name)

#define ham_def_dtor(object_type) \
	ham_impl_def_dtor(object_type)

//! @cond ignore
#define ham_impl_super_1(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME)
#define ham_impl_super_2(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME.HAM_SUPER_NAME)
#define ham_impl_super_3(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME.HAM_SUPER_NAME.HAM_SUPER_NAME)
#define ham_impl_super_4(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME HAM_REPEAT(3, .HAM_SUPER_NAME))
#define ham_impl_super_5(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME HAM_REPEAT(4, .HAM_SUPER_NAME))
#define ham_impl_super_6(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME HAM_REPEAT(5, .HAM_SUPER_NAME))
#define ham_impl_super_7(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME HAM_REPEAT(6, .HAM_SUPER_NAME))
#define ham_impl_super_8(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME HAM_REPEAT(7, .HAM_SUPER_NAME))
//! @endcond

#define ham_super_n(n, derived_ptr) HAM_CONCAT(ham_impl_super_, n)(derived_ptr)

#define ham_super(derived_ptr) ham_super_n(1, derived_ptr)

#define ham_object_method_prefix(method_name) ham_impl_obj_##method_name##_
#define ham_object_method_name(obj_name, method_name) HAM_CONCAT(ham_object_method_prefix(method_name), obj_name)

#define ham_object_vptr_prefix ham_impl_vptr_
#define ham_object_ctor_prefix ham_impl_ctor_
#define ham_object_dtor_prefix ham_impl_dtor_

#define ham_object_vptr_name(obj_name) HAM_CONCAT(ham_object_vptr_prefix, obj_name)
#define ham_object_ctor_name(obj_name) HAM_CONCAT(ham_object_ctor_prefix, obj_name)
#define ham_object_dtor_name(obj_name) HAM_CONCAT(ham_object_dtor_prefix, obj_name)

#define ham_expose_object_vptr(derived_obj) \
	ham_extern_c ham_public ham_export ham_nothrow const ham_object_vtable *ham_object_vptr_name(derived_obj)();

//! @cond ignore
#define ham_impl_define_object(obj_depth_, obj_, vtable_depth_, vtable_, ctor_, dtor_, vtable_body_) \
	ham_expose_object_vptr(obj_) \
	static ham_object *ham_object_ctor_name(obj_)(ham_object *ptr, ham_u32 nargs, va_list va){ \
		ptr->vptr = ham_object_vptr_name(obj_)(); \
		obj_ *const ret = (ctor_)((obj_*)ptr, nargs, va); \
		return ret ? ham_super_n(obj_depth_, ret) : nullptr; \
	} \
	ham_nothrow static void ham_object_dtor_name(obj_)(ham_object *obj){ \
		obj_ *const derived_ptr = (obj_*)obj; \
		(dtor_)(derived_ptr); \
	} \
	ham_nothrow const ham_object_vtable *ham_object_vptr_name(obj_)(){\
		static const ham_object_info info = (ham_object_info){ \
			.type_id = #obj_, \
			.alignment = alignof(obj_), \
			.size = sizeof(obj_), \
		}; \
		static const vtable_ ret = (vtable_){ \
			HAM_REPEAT(vtable_depth_, .HAM_SUPER_NAME) = (ham_object_vtable){ \
				.info = &info, \
				.ctor = ham_object_ctor_name(obj_), \
				.dtor = ham_object_dtor_name(obj_), \
			}, \
			HAM_EAT vtable_body_ \
		}; \
		return ham_super_n(vtable_depth_, &ret); \
	}
//! @endcond

#define ham_declare_object(obj, base) \
	typedef struct obj obj; \
	typedef struct obj##_vtable obj##_vtable; \
	typedef struct base##_vtable obj##_vtable_base; \
	typedef struct base obj##_base; \
	ham_nothrow static inline base *obj##_super(obj *ptr){ return (base*)ptr; } \
	ham_nothrow static inline const base##_vtable *obj##_vtable_super(const obj##_vtable *vptr){ return (const base##_vtable*)vptr; } \
	ham_nothrow static inline const obj##_vtable *obj##_vptr(obj *ptr){ return (const obj##_vtable*)((ham_object*)ptr)->vptr; }

#define ham_define_object_x(obj_depth, obj, vtable_depth, vtable, ctor, dtor, vtable_body) \
	ham_impl_define_object(obj_depth, obj, vtable_depth, vtable, ctor, dtor, vtable_body)

#define ham_define_object(obj, vtable, ctor, dtor, vtable_body) \
	ham_define_object_x(1, obj, 1, vtable, ctor, dtor, vtable_body)

HAM_C_API_END

#ifdef __cplusplus

#include "meta.hpp" // IWYU pragma: keep

#include <concepts>

namespace ham{
	namespace meta{
		// ham C objects

		template<typename, typename = void>
		struct is_ham_object: constant_bool<false>{};

		template<>
		struct is_ham_object<ham_object>: constant_bool<true>{};

		template<typename T>
		struct is_ham_object<T, std::void_t<decltype(T::HAM_SUPER_NAME)>>: is_ham_object<decltype(T::HAM_SUPER_NAME)>{};

		template<typename T>
		constexpr inline bool is_ham_object_v = is_ham_object<T>::value;

		template<typename T>
		concept HamObject = is_ham_object_v<T>;

		template<typename Base, typename Derived, typename Enable = void>
		struct is_ham_base_of: constant_bool<false>{};

		template<typename Base, typename Derived>
		struct is_ham_base_of<Base, Derived, std::enable_if<is_ham_object_v<Base> && is_ham_object_v<Derived>>>
			: constant_bool<
				std::is_same_v<Base, Derived> ||
				std::is_same_v<Base, decltype(Derived::HAM_SUPER_NAME)> ||
				is_ham_base_of<Base, decltype(Derived::HAM_SUPER_NAME)>::value
			>{};

		template<typename Base, typename Derived>
		constexpr inline bool is_ham_base_of_v = is_ham_base_of<Base, Derived>::value;

		template<typename Base, typename Derived>
		concept HamDerived = is_ham_base_of_v<Base, Derived>;

		template<HamObject Obj, usize N = 1, typename Enable = void>
		struct ham_object_super;

		template<HamObject Obj>
		struct ham_object_super<Obj, 0>: id<ham_object>{};

		template<HamObject Obj, usize N>
		struct ham_object_super<Obj, N>: ham_object_super<decltype(Obj::HAM_SUPER_NAME), N-1>{};

		template<HamObject Obj, usize N = 1>
		using ham_object_super_t = typename ham_object_super<Obj, N>::type;

		template<typename Object>
		struct ham_object_depth;

		template<>
		struct ham_object_depth<ham_object>: constant_usize<0>{};

		template<HamObject Obj>
		struct ham_object_depth<Obj>: constant_usize<1 + ham_object_depth<ham_object_super_t<Obj>>::value>{};

		template<HamObject Object>
		constexpr inline usize ham_object_depth_v = ham_object_depth<Object>::value;

		// ham C object vtables

		template<typename T, typename Enable = void>
		struct is_ham_object_vtable: constant_false{};

		template<>
		struct is_ham_object_vtable<ham_object_vtable>: constant_true{};

		template<typename T>
		struct is_ham_object_vtable<T, std::void_t<ham_object_super_t<T>>>: is_ham_object_vtable<ham_object_super_t<T>>{};

		template<typename T>
		constexpr inline bool is_ham_object_vtable_v = is_ham_object_vtable<T>::value;

		template<typename T>
		concept HamObjectVTable = is_ham_object_vtable_v<T>;
	}

	template<typename Obj>
	concept HamObject = meta::HamObject<Obj>;

	template<typename T>
	concept HamObjectVTable = meta::HamObjectVTable<T>;

	template<typename Base, typename Derived>
	concept HamDerived = meta::HamDerived<Base, Derived>;

	template<HamObject Obj, usize N = 1>
	static inline auto super(Obj *obj) noexcept{
		if constexpr(N == 1){
			return &obj->HAM_SUPER_NAME;
		}
		else{
			return super(&obj->HAM_SUPER_NAME);
		}
	}

	namespace detail{
		template<HamObject Obj>
		static inline Obj *object_construct_va(const ham_object_vtable *vtable, ham_object *obj, usize nargs, ...){
			va_list va;
			va_start(va, nargs);
			const auto ret = (Obj*)vtable->ctor(obj, nargs, va);
			va_end(va);
			return ret;
		}
	}

	template<HamObject Obj, typename ... Args>
	static inline Obj *object_construct(const ham_object_vtable *vtable, ham_object *obj, Args &&... args){
		return detail::object_construct_va<Obj>(vtable, obj, sizeof...(Args), args...);
	}

	namespace detail{
		template<HamObject Obj>
		struct ham_object_vtable;

		template<HamObject Obj>
		using ham_object_vtable_t = typename ham_object_vtable<Obj>::type;

		template<HamObject Base, HamDerived<Base> Derived>
		struct ham_get_base{
			static Base *get(Derived *ptr) noexcept{
				if constexpr(std::is_same_v<Derived, Base>){
					return ptr;
				}
				else if constexpr(std::is_same_v<decltype(Derived::HAM_SUPER_NAME), Base>){
					return super(ptr);
				}
				else{
					return ham_get_base<Base, decltype(Derived::HAM_SUPER_NAME)>::get(ham_super(ptr));
				}
			}
		};

		template<HamObject Object, HamObjectVTable VTable, meta::cexpr_str Name, typename Ret, typename ... Args>
		class ham_object_method_info{
			public:
				using object_type = Object;
				using vtable_type = VTable;
				using super_type  = meta::ham_object_super_t<object_type>;
				using result_type = Ret;
				using arg_types = type_tag<Args...>;

				constexpr static str8 name() noexcept{
					return str8(Name.data(), Name.len());
				}
		};

		/**
		 * @brief Object registration interface class.
		 *
		 * Given the following object definition:
		 *
		 * ```c++
		 * struct my_c_obj{
		 * 	ham_derive(ham_object)
		 * 	// ...
		 * };
		 *
		 * struct my_c_obj_vtable{
		 * 	ham_derive(ham_object_vtable)
		 * 	// ...
		 * };
		 *
		 * ham_define_object(my_c_obj, my_c_obj_vtable)
		 *
		 * using my_c_obj_registration = ham::object_registration<my_c_obj, my_c_obj_vtable>;
		 * ```
		 *
		 * The following usages are intended...
		 *
		 *
		 * At the top of a single source file place a line like the following:
		 *
		 * ```c++
		 * const auto my_c_obj_type = my_c_obj_registration::register();
		 * ```
		 *
		 * Or, from a set of library initialization/finalization routines:
		 *
		 * ```c++
		 *
		 *
		 * bool my_lib_init(){
		 * 	my_c_obj_registration::register();
		 * }
		 *
		 * void my_lib_fini(){
		 * 	my_c_obj_registration::deregister();
		 * }
		 * ```
		 */
		template<
			HamObject Object,
			HamObjectVTable VTable,
			typename ... MethodInfos
		>
		struct ham_object_registration{
			public:
				using object_type = Object;
				using vtable_type = VTable;
				using super_type  = meta::ham_object_super_t<object_type>;

				using object_ptr       = object_type*;
				using object_const_ptr = const object_type*;
				using vtable_ptr       = const vtable_type*;

				static bool register_(){ return false; }
		};
	}

	template<typename T, typename ... Args>
	concept Callable = requires(const T &t){ t(std::declval<Args>()...); };

	template<typename Object, meta::cexpr_str Name, typename Sig, auto Ptr>
	class object_method;

	template<typename Object, meta::cexpr_str Name, typename Ret, typename ... Args, auto Ptr>
		requires Callable<decltype(Ptr), Object*, Args...> || Callable<decltype(Ptr), const Object*, Args...>
	class object_method<Object, Name, Ret(Args...), Ptr>{
		public:
			using result_type = Ret;
			using param_types = type_tag<Args...>;
			using pointer = decltype(Ptr);

			constexpr static bool is_const = Callable<decltype(Ptr), const Object*, Args...>;

			constexpr static str8 name() noexcept{ return Name; }

			constexpr static pointer ptr() noexcept{ return Ptr; }

			template<
				bool IsConst = is_const,
				std::enable_if_t<!IsConst, int> = 0,
				typename ... UArgs
			>
			constexpr static decltype(auto) call(Object *self, UArgs &&... args) noexcept(noexcept(Ptr(std::forward<UArgs>(args)...))){
				if constexpr(std::is_void_v<Ret>){
					Ptr(self, std::forward<UArgs>(args)...);
				}
				else{
					return Ptr(self, std::forward<UArgs>(args)...);
				}
			}

			template<
				bool IsConst = is_const,
				std::enable_if_t<IsConst, int> = 0,
				typename ... UArgs
			>
			constexpr static decltype(auto) call(const Object *self, UArgs &&... args) noexcept(noexcept(Ptr(std::forward<UArgs>(args)...))){
				if constexpr(std::is_void_v<Ret>){
					Ptr(self, std::forward<UArgs>(args)...);
				}
				else{
					return Ptr(self, std::forward<UArgs>(args)...);
				}
			}
	};

	namespace detail{
		template<typename Object, typename ... Methods>
		struct object_builder{
			static constexpr std::array<str8, sizeof...(Methods)> method_names() noexcept{
				return { Methods::name()... };
			}

			static constexpr std::tuple<typename Methods::pointer...> method_ptrs() noexcept{
				return std::make_tuple(Methods::ptr()...);
			}

			static const ::ham_object_info *object_info() noexcept{
				static const ::ham_object_info ret{
					.type_id = meta::type_name_v<Object>,
					.alignment = alignof(Object),
					.size = sizeof(Object),
				};
				return &ret;
			}

			static ham_object *object_ctor(ham_object *obj, ham_u32 nargs, va_list va){
				return new(obj) Object();
			}

			static void object_dtor(ham_object *obj){
				const auto actual_ptr = reinterpret_cast<Object*>(obj);
				actual_ptr->~Object();
			}

			struct vtable{
				vtable(){
					::ham_object_vtable *super = ham_super(this);
					super->info = object_info();
					super->ctor = object_ctor;
					super->dtor = object_dtor;
					methods = method_ptrs();
				}

				ham_derive(::ham_object_vtable);
				std::tuple<typename Methods::pointer...> methods;
			};

			static const ::ham_object_vtable *object_vptr() noexcept{
				static const vtable ret;
				return ham_super(&ret);
			}
		};

		template<meta::cexpr_str QueryName, typename ... Methods>
		struct object_method_query;

		template<meta::cexpr_str QueryName, typename Head, typename ... Tail>
		struct object_method_query<QueryName, Head, Tail...>: object_method_query<QueryName, Tail...>{};

		template<meta::cexpr_str QueryName, typename Object, typename Ret, typename ... Args, auto Ptr, typename ... Methods>
		struct object_method_query<QueryName, object_method<Object, QueryName, Ret(Args...), Ptr>, Methods...>{
			using type = object_method<Object, QueryName, Ret(Args...), Ptr>;
		};
	}

	template<typename Impl, typename Object>
	class object_interface{};

	template<HamObject Object>
	class object_view: public object_interface<object_view<Object>, Object>{
		public:
			using object_type = Object;
			using vtable_type = detail::ham_object_vtable_t<object_type>;

			using interface_type = object_interface<object_view<Object>, Object>;

			template<HamDerived<Object> Derived>
			object_view(Derived *ptr_) noexcept
				: m_obj(detail::ham_get_base<Object, Derived>::get(ptr_)){}

			auto super() const noexcept -> std::enable_if_t<!std::is_same_v<Object, ham_object>, object_view<meta::ham_object_super_t<Object>>>{
				return &m_obj->HAM_SUPER_NAME;
			}

			interface_type *operator->() noexcept{ return this; }
			const interface_type *operator->() const noexcept{ return this; }

			const vtable_type *vtable() const noexcept{ return detail::ham_get_base<ham_object, Object>::get(m_obj)->vtable; }

			template<HamDerived<Object> Derived>
			object_view<Derived> dyn_cast(){
				// TODO: implement proper type information
				return nullptr;
			}

		private:
			object_type *m_obj;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_OBJECT_H
