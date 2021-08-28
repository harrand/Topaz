#ifndef TOPAZ_GL_IMPL_COMMON_SHADER_HPP
#define TOPAZ_GL_IMPL_COMMON_SHADER_HPP
#include <string>
#include <unordered_map>
#include <optional>

namespace tz::gl
{
    enum class ShaderType
    {
        VertexShader,
        FragmentShader,
        ComputeShader
    };

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
}

#endif // TOPAZ_GL_IMPL_COMMON_SHADER_HPP