#ifndef HAM_STD_VECTOR_HPP
#define HAM_STD_VECTOR_HPP 1

/**
 * @defgroup HAM_STD_VECTOR std::vector wrapper
 * @ingroup HAM
 * @{
 */

#include "memory.h"

#include <vector>

namespace ham{
	template<typename T, typename Allocator = allocator<T>>
	using std_vector = std::vector<T, Allocator>;
}

/**
 * @}
 */

#endif // !HAM_STD_VECTOR_HPP
