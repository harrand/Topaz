#include "engine_meta.hpp"

PropertiesFile::PropertiesFile(std::string properties_path): MDLFile(properties_path)
{

}

bool PropertiesFile::has_inline_resources() const
{
    return !this->exists_tag(tz::meta::resources_tag) || this->get_tag(tz::meta::resources_tag) == "this";
}

ResourcesFile::ResourcesFile(std::string resources_path): MDLFile(resources_path)
{

}

EngineMeta::EngineMeta(): properties(tz::meta::default_properties_filename), resources({})
{
    this->properties.clear();
    this->properties.add_tag(tz::meta::resources_tag, tz::meta::default_inline_resources_filename);
    this->properties.update();
}

/// Initialization list didn't like ternary operator so doing it in constructor body grudgingly.
EngineMeta::EngineMeta(const std::string& properties_path): properties(properties_path), resources({})
{
    if(!this->properties.has_inline_resources())
        this->resources = ResourcesFile(this->properties.get_tag(tz::meta::default_external_resources_filename));
}

const PropertiesFile& EngineMeta::get_properties() const
{
    return this->properties;
}

const ResourcesFile& EngineMeta::get_resources() const
{
    return this->resources.has_value() ? this->resources.value() : dynamic_cast<const ResourcesFile&>(this->properties);
}