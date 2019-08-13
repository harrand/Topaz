//
// Created by Harrand on 13/08/2019.
//

#ifndef TOPAZ_ATTRIBUTE_COLLECTION_HPP
#define TOPAZ_ATTRIBUTE_COLLECTION_HPP
#include <initializer_list>
#include <vector>
#include <string>
#include "core/topaz.hpp"

class ShaderAttributeCollection
{
public:
    using AttributeCollection = std::vector<std::string>;
    using iterator = AttributeCollection::iterator;
    using const_iterator = AttributeCollection::const_iterator;

    ShaderAttributeCollection(std::initializer_list<std::string> attributes =
    {
            tz::consts::graphics::mesh::attribute::position_attribute,
            tz::consts::graphics::mesh::attribute::texcoord_attribute,
            tz::consts::graphics::mesh::attribute::normal_attribute,
            tz::consts::graphics::mesh::attribute::tangent_attribute,
            tz::consts::graphics::mesh::attribute::instance_model_x_attribute,
            tz::consts::graphics::mesh::attribute::instance_model_y_attribute,
            tz::consts::graphics::mesh::attribute::instance_model_z_attribute,
            tz::consts::graphics::mesh::attribute::instance_model_w_attribute
    });
    std::size_t get_size() const;
    iterator begin();
    const_iterator cbegin() const;
    iterator end();
    const_iterator cend() const;
    std::string& operator[](std::size_t index);
    const std::string& operator[](std::size_t index) const;
private:
    AttributeCollection attributes;
};


#endif //TOPAZ_ATTRIBUTE_COLLECTION_HPP
