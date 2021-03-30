#include <algorithm>
#include <type_traits>

namespace tz::algo
{
	template<typename StandardContainer>
	constexpr std::size_t sizeof_element([[maybe_unused]] const StandardContainer& container)
	{
		return sizeof_element<StandardContainer>();
	}

	template<typename StandardContainer>
	constexpr std::size_t sizeof_element()
	{
		return sizeof(typename StandardContainer::value_type);
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