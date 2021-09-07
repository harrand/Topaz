#ifndef TOPAZ_GL_IMPL_COMMON_SHADER_HPP
#define TOPAZ_GL_IMPL_COMMON_SHADER_HPP
#include <string>
#include <unordered_map>
#include <optional>
#include "gl/api/shader.hpp"

namespace tz::gl
{
    enum class ShaderMetaValue
    {
        // Buffers
        UBO,
        SSBO,
        // Textures
        Texture,

        Count
    };

    namespace detail
    {
        constexpr const char* meta_value_names[]{"ubo", "ssbo", "texture"};
    }

    /**
     * @brief ShaderMetas contain information about a specific resource ID.
     * A metadata string is interpreted as a sequence of the following expressions, separated by newline:
     * "res-id = value"
     * Where res-id is the id of some resource associated with a renderer, and value represents some information about the resource.
     * For example, if "0 = ssbo" is written, this implies the resource with id 0 is not only a buffer resource, but also to be interpreted as an SSBO by the renderer.
     */
    class ShaderMeta
    {
    public:
        static ShaderMeta from_metadata_string(const std::string& metadata);
        ShaderMeta() = default;

        std::optional<ShaderMetaValue> try_get_meta_value(unsigned int resource_id) const;
    private:
        std::unordered_map<unsigned int, ShaderMetaValue> resource_types = {};
    };

    class ShaderBuilderBase : public IShaderBuilder
    {
    public:
        ShaderBuilderBase() = default;
        virtual void set_shader_file(ShaderType type, std::filesystem::path shader_file) override;
        virtual void set_shader_source(ShaderType type, std::string source_code) override;
        virtual void set_shader_meta(ShaderType type, std::string metadata) override;
        virtual std::string_view get_shader_source(ShaderType type) const override;
        virtual std::string_view get_shader_meta(ShaderType type) const override;
        virtual bool has_shader(ShaderType type) const override;
    private:
        struct ShaderInfo
        {
            static ShaderInfo null(){return {.source = "", .metadata = ""};}
            std::string source;
            std::string metadata;
        };
        ShaderInfo vertex;
        ShaderInfo fragment;
        ShaderInfo compute;
    };
}

#endif // TOPAZ_GL_IMPL_COMMON_SHADER_HPP