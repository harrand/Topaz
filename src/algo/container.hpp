#ifndef TOPAZ_ALGO_CONTAINER_HPP
#define TOPAZ_ALGO_CONTAINER_HPP
#include <cstddef>

namespace tz::algo
{

	/**
	 * \addtogroup tz_algo Topaz Algorithms Library (tz::algo)
	 * Contains common algorithms used in Topaz modules that aren't available in the C++ standard library.
	 * @{
	 */

	/**
	 * Retrieve the size of an element in the given standard container.
	 * Precondition: StandardContainer must define the member alias 'value_type'. Otherwise, usage will fail to compile.
	 * @tparam StandardContainer Type representing a standard container. Examples: std::vector<int>, tz::mem::UniformPool<float>.
	 * @param container Unused container value. Only pass a parameter if you wish for type deduction to take place.
	 * @return Size of an element in the container, in bytes. For example: for StandardContainer std::vector<int>, expect this to return sizeof(int).
	 */
	template<typename StandardContainer>
	constexpr std::size_t sizeof_element(const StandardContainer& container);

	/**
	 * Retrieve the size of an element in the given standard container.
	 * Precondition: StandardContainer must define the member alias 'value_type'. Otherwise, usage will fail to compile.
	 * Note: As this has no parameters, the type cannot be deduced. You must explicitly state the type to use this overload. If you wish to utilise type deduction, see sizeof_element(const StandardContainer&).
	 * @tparam StandardContainer Type representing a standard container. Examples: std::vector<int>, tz::mem::UniformPool<float>.
	 * @return Size of an element in the container, in bytes. For example: for StandardContainer std::vector<int>, expect this to return sizeof(int).
	 */
	template<typename StandardContainer>
	constexpr std::size_t sizeof_element();

	/**
	 * @}
	 */
}

#include "algo/container.inl"
#endif // TOPAZ_ALGO_CONTAINER_HPP