#ifndef TOPAZ_CORE_CONTAINERS_ENUM_FIELD_HPP
#define TOPAZ_CORE_CONTAINERS_ENUM_FIELD_HPP
#include "core/types.hpp"
#include <initializer_list>
#include <vector>

namespace tz
{

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
        EnumField<E>& operator|=(E type);
        EnumField<E> operator|(E type) const;

        bool operator==(const EnumField<E>& rhs) const = default;
        explicit operator E() const;
    private:
        std::vector<E> elements;
    };
}

#include "core/containers/enum_field.inl"
#endif // TOPAZ_CORE_CONTAINERS_ENUM_FIELD_HPP