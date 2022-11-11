/*
 * Ham Runtime
 * Copyright (C) 2022 Keith Hammond
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

#ifndef HAM_COLONY_H
#define HAM_COLONY_H 1

/**
 * @defgroup HAM_COLONY Colony data-structure
 * Named after the beautiful <a href="https://plflib.org/colony.htm">plf::colony</a>.
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_colony ham_colony;

ham_api ham_colony *ham_colony_create(ham_usize obj_alignment, ham_usize obj_size);

ham_api ham_nothrow void ham_colony_destroy(ham_colony *colony);

ham_api void *ham_colony_emplace(ham_colony *colony);

ham_api ham_nothrow bool ham_colony_erase(ham_colony *colony, void *ptr);

ham_api ham_nothrow bool ham_colony_contains(const ham_colony *colony, const void *ptr);

ham_api ham_nothrow bool ham_colony_compact(ham_colony *colony);

typedef bool(*ham_colony_iterate_fn)(void *ptr, void *user);

ham_api ham_usize ham_colony_iterate(ham_colony *colony, ham_colony_iterate_fn fn, void *user);

ham_api ham_nothrow ham_usize ham_colony_num_elements(const ham_colony *colony);

ham_api ham_nothrow void *ham_colony_element_at(ham_colony *colony, ham_usize idx);

ham_api bool ham_colony_view_erase(ham_colony *colony, void *ptr, ham_colony_iterate_fn view_fn, void *user);

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	class colony_create_error: public exception{
		public:
			const char *api() const noexcept{ return "ham::colony"; }
			const char *what() const noexcept{ return "Error in ham_colony_create"; }
	};

	template<typename T = void>
	class colony{
		public:
			template<
				bool HasDefaultCtor = !std::is_same_v<T, void>,
				std::enable_if_t<HasDefaultCtor, int> = 0
			>
			colony()
				: m_handle(ham_colony_create(alignof(T), sizeof(T)))
			{
				if(!m_handle) throw colony_create_error{};
			}

			template<
				bool HasSizedCtor = std::is_same_v<T, void>,
				std::enable_if_t<HasSizedCtor, int> = 0
			>
			colony(usize elem_alignment, usize elem_size)
				: m_handle(ham_colony_create(elem_alignment, elem_size))
			{
				if(!m_handle) throw colony_create_error{};
			}

			colony(colony&&) noexcept = default;

			~colony(){
				if constexpr(!std::is_same_v<T, void>){
					if(m_handle){
						ham_colony_iterate(
							m_handle.get(),
							[](void *ptr, void*){ std::destroy_at((T*)ptr); return true; },
							nullptr
						);
					}
				}
			}

			colony &operator=(colony&&) noexcept = default;

			usize size() const noexcept{ return ham_colony_num_elements(m_handle.get()); }

			T *at(usize idx) noexcept{ return reinterpret_cast<T*>(ham_colony_element_at(m_handle.get(), idx)); }

			template<typename ... Args>
			T *emplace(Args &&... args){
				const auto mem = ham_colony_emplace(m_handle.get());
				if(!mem) return nullptr;

				if constexpr(std::is_same_v<T, void>){
					static_assert(sizeof...(Args) == 0);
					return mem;
				}
				else{
					return new(mem) T(std::forward<Args>(args)...);
				}
			}

			bool erase(T *ptr) noexcept{
				return ham_colony_view_erase(m_handle.get(), ptr, [](void *ptr, void*){ std::destroy_at((T*)ptr); }, nullptr);
			}

			bool compact() noexcept{ return ham_colony_compact(m_handle.get()); }

			usize iterate(ham_colony_iterate_fn fn, void *user){
				return ham_colony_iterate(m_handle.get(), fn, user);
			}

		private:
			unique_handle<ham_colony*, ham_colony_destroy> m_handle;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_COLONY_H
