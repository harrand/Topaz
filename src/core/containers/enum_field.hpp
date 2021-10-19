#ifndef TOPAZ_CORE_CONTAINERS_ENUM_FIELD_HPP
#define TOPAZ_CORE_CONTAINERS_ENUM_FIELD_HPP
#include "core/types.hpp"
#include <initializer_list>
#include <vector>

namespace tz
{
	/**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */
	template<tz::EnumClass E>
	class EnumField
	{
	public:
		constexpr EnumField() = default;
		constexpr EnumField(std::initializer_list<E> types);
		constexpr EnumField(E type);
		constexpr ~EnumField() = default;
		bool contains(E type) const;
		bool contains(const EnumField<E>& field) const;
		std::size_t count() const;
		bool empty() const;
		EnumField<E>& operator|=(E type);
		EnumField<E> operator|(E type) const;
		auto begin() const;
		auto begin();
		auto end() const;
		auto end();

		bool operator==(const EnumField<E>& rhs) const = default;
		explicit operator E() const;
	private:
		std::vector<E> elements;
	};

	/**
	 * @}
	 */
}

#include "core/containers/enum_field.inl"
#endif // TOPAZ_CORE_CONTAINERS_ENUM_FIELD_HPP
