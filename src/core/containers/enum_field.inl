#include <algorithm>

namespace tz
{
    template<tz::EnumClass E>
    constexpr EnumField<E>::EnumField(E type):
    elements(type)
    {}

    template<tz::EnumClass E>
    constexpr EnumField<E>::EnumField(std::initializer_list<E> types):
    elements(types)
    {}

    template<tz::EnumClass E>
    bool EnumField<E>::contains(E type) const
    {
        return std::find(this->elements.begin(), this->elements.end(), type) != this->elements.end();
    }

    template<tz::EnumClass E>
    bool EnumField<E>::contains(const EnumField<E>& field) const
    {
        for(E type : field.elements)
        {
            if(!this->contains(type))
            {
                return false;
            }
        }
        return true;
    }

    template<tz::EnumClass E>
    EnumField<E>& EnumField<E>::operator|=(E type)
    {
        if(!this->contains(type))
        {
            this->elements.push_back(type);
        }
        return *this;
    }

    template<tz::EnumClass E>
    EnumField<E> EnumField<E>::operator|(E type) const
    {
        EnumField<E> cpy = *this;
        return cpy |= type;
    }
}