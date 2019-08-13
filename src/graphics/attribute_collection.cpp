//
// Created by Harrand on 13/08/2019.
//

#include "attribute_collection.hpp"

ShaderAttributeCollection::ShaderAttributeCollection(std::initializer_list<std::string> attributes): attributes(attributes){}

std::size_t ShaderAttributeCollection::get_size() const
{
    return this->attributes.size();
}

ShaderAttributeCollection::iterator ShaderAttributeCollection::begin()
{
    return this->attributes.begin();
}

ShaderAttributeCollection::const_iterator ShaderAttributeCollection::cbegin() const
{
    return this->attributes.cbegin();
}

ShaderAttributeCollection::iterator ShaderAttributeCollection::end()
{
    return this->attributes.end();
}

ShaderAttributeCollection::const_iterator ShaderAttributeCollection::cend() const
{
    return this->attributes.cend();
}

std::string& ShaderAttributeCollection::operator[](std::size_t index)
{
    topaz_assert(this->attributes.size() > index, "ShaderAttributeCollection::operator[]: Index ", index, "out of range (Size == ", this->attributes.size(), ")");
    return this->attributes[index];
}

const std::string& ShaderAttributeCollection::operator[](std::size_t index) const
{
    topaz_assert(this->attributes.size() > index, "ShaderAttributeCollection::operator[]: Index ", index, "out of range (Size == ", this->attributes.size(), ")");
    return this->attributes[index];
}