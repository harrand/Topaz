#ifndef ENGINE_META_HPP
#define ENGINE_META_HPP
#include "MDL/mdl_file.hpp"

namespace tz::meta
{
    constexpr char default_properties_filename[] = "properties.mdl";
    constexpr char default_external_resources_filename[] = "resources.mdl";
    constexpr char default_inline_resources_filename[] = "inline";
    constexpr char resources_tag[] = "resources";

    constexpr char default_shader_tag_name[] = "default_shader";
    constexpr char default_gui_shader_tag_name[] = "default_gui_shader";
    constexpr char default_scene_tag_name[] = "default_scene";
}

enum class StandardProperty
{
    DEFAULT_SHADER,
    DEFAULT_GUI_SHADER,
    DEFAULT_SCENE,
};

class PropertiesFile : public MDLFile
{
public:
    explicit PropertiesFile(std::string properties_path);
    /**
     * Returns true if any of the following are true:
     *  - There is no 'resources' tag
     *  - The 'resources' tag has a value exactly equal to 'this'
     * @return - Whether the Properties has inline Resources declared.
     */
    bool has_inline_resources() const;
    std::string get_property_path(StandardProperty property) const;
private:
};

class ResourcesFile : public MDLFile
{
public:
    explicit ResourcesFile(std::string resources_path);
};

/**
 * EngineMeta is contains two elements:
 * Properties and Resources.
 *
 * Properties is a file which contains metadata about the Topaz implementation.
 * Specifications such as the default-shader, the default-gui-shader and the Resources location are made here.
 * Properties are normally defined by the filename 'properties.mdl'
 *
 * Resources is a file which specifies all internal assets.
 * Assets can be any file used in a Topaz project, like textures or 3D models.
 * Internal assets are instantiated upon initialisation of Topaz, and are available for use at any time.
 * External assets are files which are not specified in Resources and thus are not tracked or handled by Topaz; you must handle them yourself.
 * Although external assets can be used for all actions, internal assets are recommended.
 * Resources are normally defined either inline in Properties or externally by the filename 'resources.mdl'. The path to the Resources should always be specified in Properties.
 *
 * Footnote: Any relative-path specifications made in Properties and Resources should be relative to the file, not the binary file.
 */
class EngineMeta
{
public:
    /**
     * Default constructor for EngineMeta.
     * Creates 'properties.mdl' in the same directory as the program location.
     * Be warned: If 'properties.mdl' already exists in this directory, it WILL be overwritten.
     */
    EngineMeta();
    /**
     * Constructs an EngineMeta from an existing Properties file. The Resources file is instantiated in accordance to whatever specified in the Properties.
     * @param properties_path - Path to an existing Properties file.
     */
    EngineMeta(const std::string& properties_path);
    const PropertiesFile& get_properties() const;
    /**
     * Get the Resources, regardless whether it is external or inline.
     * @return - Const-reference to the Resources MDLFile if there is one. If not, the return value refers (polymorphically) to the Properties which must have inline Resources.
     */
    const ResourcesFile& get_resources() const;
private:
    PropertiesFile properties;
    std::optional<ResourcesFile> resources;
};

#endif //ENGINE_META_HPP