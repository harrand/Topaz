#ifndef TOPAZ_GL_API_SHADER_HPP
#define TOPAZ_GL_API_SHADER_HPP
#include "gl/impl/frontend/common/shader.hpp"
#include <string>
#include <string_view>
#include <filesystem>

namespace tz::gl
{
    /**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * A collection of low-level renderer-agnostic graphical interfaces.
	 * @{
	 */
    class IShaderBuilder
    {
    public:
        virtual void set_shader_file(ShaderType type, std::filesystem::path shader_file) = 0;
        virtual void set_shader_source(ShaderType type, std::string source_code) = 0;
        virtual void set_shader_meta(ShaderType type, std::string metadata) = 0;
        virtual std::string_view get_shader_source(ShaderType type) const = 0;
        virtual std::string_view get_shader_meta(ShaderType type) const = 0;
        virtual bool has_shader(ShaderType type) const = 0;
    };

    class IShader
    {
    public:
        virtual const ShaderMeta& get_meta() const = 0;
    };
    /**
     * @}
     */
}

#endif // TOPAZ_GL_API_SHADER_HPP