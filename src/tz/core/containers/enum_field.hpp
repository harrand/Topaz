#ifndef TOPAZ_CORE_CONTAINERS_ENUM_FIELD_HPP
#define TOPAZ_CORE_CONTAINERS_ENUM_FIELD_HPP
#include "tz/core/types.hpp"
#include <initializer_list>
#include <vector>

namespace tz
{
	/**
	 * @ingroup tz_core_containers
	 * Container for enum values, useful for vintage bitfield types.
	 * @tparam E Enum class type.
	 */
	template<tz::EnumClass E>
	class EnumField
	{
	public:
		/**
		 * Create an empty field.
		 */
		constexpr EnumField() = default;
		/**
		 * Create a field from the provided values.
		 */
		constexpr EnumField(std::initializer_list<E> types);
		/**
		 * Create a field from a single value.
		 */
		constexpr EnumField(E type);
		constexpr ~EnumField() = default;
		/**
		 * Query as to whether the field contains the given value.
		 * @note The value is only contained if the exact value was inserted into the field, not if the field contains elements forming a bitwise equivalent.
		 *
		 * Example: A = {0x01, 0x10}, B = {0x00, 0x11}
		 * `A.contains(0x11) == false`, `B.contains(0x11) == true`.
		 * @return True if value is within the field, false otherwise.
		 */
		bool contains(E type) const;
		/**
		 * Query as to whether the field contains each element within another field.
		 * @return True if for each element `x` in the parameter field, `this->contains(x)`, otherwise false.
		 */
		bool contains(const EnumField<E>& field) const;
		/**
		 * Retrieve the number of elements within the field.
		 * @return Number of elements within the field.
		 */
		std::size_t count() const;
		/**
		 * Query as to whether the field is empty or not.
		 * @return True if `count() == 0`, otherwise false.
		 */
		bool empty() const;
		/**
		 * Add the element to the field. The cumulative value `C` of the field becomes `C | E`.
		 * @param type Element to add to the field.
		 * @return This.
		 */
		EnumField<E>& operator|=(E type);
		/**
		 * Create a copy of this field, but with the parameter element added. The cumulative value `C` of the field becomes `C | E`.
		 * @param type Element to add to the new field.
		 * @return A copy of this.
		 */
		EnumField<E> operator|(E type) const;
		/**
		 * Retrieve an iterator to the beginning of the elements.
		 */
		auto begin() const;
		/**
		 * Retrieve an iterator to the beginning of the elements.
		 */
		auto begin();
		/**
		 * Retrieve an iterator to the end of the elements.
		 */
		auto end() const;
		/**
		 * Retrieve an iterator to the end of the elements.
		 */
		auto end();
		/**
		 * Retrieve the first value within the field.
		 * @pre Field must not be empty, otherwise the behaviour is undefined.
		 * @return First element of the field.
		 */
		const E& front() const;
		/**
		 * Retrieve the last value within the field.
		 * @pre Field must not be empty, otherwise the behaviour is undefined.
		 * @return Last element of the field.
		 */
		const E& back() const;
		/**
		 * Query as to whether the elements of the field exactly match that of another field. This may return false even if the cumulative values are equivalent.
		 * @return True if fields contain same elements, otherwise false.
		 */
		bool operator==(const EnumField<E>& rhs) const = default;
		/**
		 * Retrieve the cumulative value of the field.
		 */
		explicit operator E() const;
	private:
		std::vector<E> elements;
	};
}

#include "tz/core/containers/enum_field.inl"
#endif // TOPAZ_CORE_CONTAINERS_ENUM_FIELD_HPP
