#ifndef TOPAZ_ALGO_CONTAINER_HPP
#define TOPAZ_ALGO_CONTAINER_HPP
#include <cstddef>
#include <concepts>

namespace tz::algo
{

	/**
	 * \addtogroup tz_algo Topaz Algorithms Library (tz::algo)
	 * Contains common algorithms used in Topaz modules that aren't available in the C++ standard library.
	 * @{
	 */

	template<typename IteratorType>
	concept Iterator = requires(IteratorType a)
	{
		a++;
		++a;
		a--;
		--a;
		*a;
	};

	template<typename ContainerType>
	concept Iterable = requires(ContainerType a)
	{
		{a.begin()} -> Iterator;
		{a.end()} -> Iterator;
	};

	template<typename Container>
	concept UniformValueContainer = requires
	{
		typename Container::value_type;
	};

	/**
	 * Retrieve the size of an element in the given standard container.
	 * Precondition: StandardContainer must define the member alias 'value_type'. Otherwise, usage will fail to compile.
	 * @tparam StandardContainer Type representing a standard container. Examples: std::vector<int>, tz::mem::UniformPool<float>.
	 * @param container Unused container value. Only pass a parameter if you wish for type deduction to take place.
	 * @return Size of an element in the container, in bytes. For example: for StandardContainer std::vector<int>, expect this to return sizeof(int).
	 */
	template<UniformValueContainer Container> 
	constexpr std::size_t sizeof_element(const Container& container);

	/**
	 * Retrieve the size of an element in the given standard container.
	 * Precondition: StandardContainer must define the member alias 'value_type'. Otherwise, usage will fail to compile.
	 * Note: As this has no parameters, the type cannot be deduced. You must explicitly state the type to use this overload. If you wish to utilise type deduction, see sizeof_element(const StandardContainer&).
	 * @tparam StandardContainer Type representing a standard container. Examples: std::vector<int>, tz::mem::UniformPool<float>.
	 * @return Size of an element in the container, in bytes. For example: for StandardContainer std::vector<int>, expect this to return sizeof(int).
	 */
	template<UniformValueContainer Container>
	constexpr std::size_t sizeof_element();

	/**
	 * Query as to whether a given container contains a certain element.
	 * @tparam StandardContainer Type of the container. Must support StandardContainer::begin and StandardContainer::end.
	 * @tparam ElementType Type of the element. StandardContainer must be iterable with respect to ElementType.
	 * @param container Container to check the element exists.
	 * @param element Element value to check if exists within container.
	 */
	template<typename StandardContainer, typename ElementType>
	bool contains_element(const StandardContainer& container, const ElementType& element);

	/**
	 * Query as to whether a container contains a predicated value.
	 * @tparam StandardContainer Type of the container. Must support StandardContainer::begin and StandardContainer::end.
	 * @tparam ConvertibleToElement Type of the value to check exists within the StandardContainer. Does not need to be iterable with respect to StandardContainer.
	 * @tparam ToElementFunctor Invokable type of signature `ToElementFunctor(ConvertibleToElement)->StandardContainer::element_type`.
	 * @param container Container to check the element exists.
	 * @param element Element value to check if exists within container.
	 * @param functor Functor to retrieve an iterable value from the given element.
	 */
	template<typename StandardContainer, typename ConvertibleToElement, typename ToElementFunctor>
	bool contains_element(const StandardContainer& container, const ConvertibleToElement& element, const ToElementFunctor& functor);

	/**
	 * @}
	 */
}

#include "algo/container.inl"
#endif // TOPAZ_ALGO_CONTAINER_HPP