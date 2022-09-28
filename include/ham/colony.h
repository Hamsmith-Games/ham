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

ham_api bool ham_colony_view_erase(ham_colony *colony, void *ptr, ham_colony_iterate_fn view_fn, void *user);

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	template<typename T>
	class colony{
		public:
			colony()
				: m_handle(ham_colony_create(alignof(T), sizeof(T)))
			{}

			colony(colony&&) noexcept = default;

			~colony(){
				if(m_handle){
					ham_colony_iterate(
						m_handle.get(),
						[](void *ptr, void*){ std::destroy_at((T*)ptr); },
						nullptr
					);
				}
			}

			colony &operator=(colony&&) noexcept = default;

			template<typename ... Args>
			T *emplace(Args &&... args){
				const auto mem = ham_colony_emplace(m_handle.get());
				if(!mem) return nullptr;

				return new(mem) T(std::forward<Args>(args)...);
			}

			bool erase(T *ptr) noexcept{
				return ham_colony_view_erase(m_handle.get(), ptr, [](void *ptr, void*){ std::destroy_at((T*)ptr); }, nullptr);
			}

			bool compact() noexcept{ return ham_colony_compact(m_handle.get()); }

		private:
			unique_handle<ham_colony*, ham_colony_destroy> m_handle;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_COLONY_H
