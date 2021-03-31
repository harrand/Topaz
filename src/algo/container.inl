#include <algorithm>
#include <type_traits>

namespace tz::algo
{
	template<UniformValueContainer Container>
	constexpr std::size_t sizeof_element([[maybe_unused]] const Container& container)
	{
		return sizeof_element<Container>();
	}

	template<UniformValueContainer Container> 
	constexpr std::size_t sizeof_element()
	{
		return sizeof(typename Container::value_type);
	}

	template<typename StandardContainer, typename ElementType>
	bool contains_element(const StandardContainer& container, const ElementType& element)
	{
		return std::find(container.begin(), container.end(), element) != container.end();
	}

	template<typename StandardContainer, typename ConvertibleToElement, typename ToElementFunctor>
	bool contains_element(const StandardContainer& container, const ConvertibleToElement& element, const ToElementFunctor& functor)
	{
		return contains_element<StandardContainer>(container, functor(element));
	}
}