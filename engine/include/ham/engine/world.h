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

#ifndef HAM_ENGINE_WORLD_H
#define HAM_ENGINE_WORLD_H 1

/**
 * @defgroup HAM_ENGINE_WORLD World management
 * @ingroup HAM_ENGINE
 * @{
 */

#include "ham/engine/config.h"

#include "ham/net.h" // IWYU pragma: keep

#include "entity.h"

HAM_C_API_BEGIN

typedef struct ham_world ham_world;

//! A single world partition, part of a larger world tree
typedef struct ham_world_partition ham_world_partition;

ham_engine_api ham_world *ham_world_create(ham_str8 name);

ham_engine_api void ham_world_destroy(ham_world *world);

ham_engine_api ham_entity *ham_entity_vcreate(ham_world *world, const ham_entity_vtable *ent_vt, ham_u32 nargs, va_list va);

ham_engine_api void ham_entity_destroy(ham_entity *ent);

//! @cond ignore
ham_used static inline ham_entity *ham_impl_entity_create(ham_world *world, const ham_entity_vtable *ent_vt, ham_u32 nargs, ...){
	va_list va;
	va_start(va, nargs);
	ham_entity *const ret = ham_entity_vcreate(world, ent_vt, nargs, va);
	va_end(va);
	return ret;
}
//! @endcond

#define ham_entity_create(world, ent_vt, ...) \
	ham_impl_entity_create((world), (ent_vt), HAM_NARGS(__VA_ARGS__) __VA_OPT__(,) __VA_ARGS__)

HAM_C_API_END

#ifdef __cplusplus

namespace ham::engine{
	template<typename ... Tags>
	class basic_world_view{
		public:
			static constexpr bool is_mutable = ham::meta::type_list_contains_v<meta::type_list<Tags...>, mutable_tag>;

			using pointer = std::conditional_t<is_mutable, ham_world*, const ham_world*>;

			basic_world_view(pointer ptr_) noexcept
				: m_ptr(ptr_){}

			operator pointer() const noexcept{ return m_ptr; }

			pointer ptr() const noexcept{ return m_ptr; }

		private:
			pointer m_ptr;
	};

	using world_view = basic_world_view<mutable_tag>;
	using const_world_view = basic_world_view<>;
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_ENGINE_WORLD_H
