#if TZ_VULKAN
#include "gl/vk/impl/hardware/queue_family.hpp"

namespace tz::gl::vk::hardware
{
    QueueFamilyTypeField::QueueFamilyTypeField(std::initializer_list<QueueFamilyType> types):
    supported_types(types){}

    QueueFamilyTypeField& QueueFamilyTypeField::operator|=(QueueFamilyType type)
    {
        if(!this->contains(type))
        {
            this->supported_types.push_back(type);
        }
        return *this;
    }

    QueueFamilyTypeField QueueFamilyTypeField::operator|(QueueFamilyType type) const
    {
        QueueFamilyTypeField cpy = *this;
        return cpy |= type;
    }

    bool QueueFamilyTypeField::contains(QueueFamilyType type) const
    {
        return std::find(this->supported_types.begin(), this->supported_types.end(), type) != this->supported_types.end();
    }

    bool QueueFamilyTypeField::contains(QueueFamilyTypeField field) const
    {
        for(const QueueFamilyType& type : field.supported_types)
        {
            if(!this->contains(type))
            {
                return false;
            }
        }
        return true;
    }
}

#endif