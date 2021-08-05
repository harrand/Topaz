#ifndef TOPAZ_GL_OUTPUT_HPP
#define TOPAZ_GL_OUTPUT_HPP
#include "gl/api/renderer.hpp"
#include "gl/component.hpp"
#include <vector>

namespace tz::gl
{
    class TextureOutput : public IRendererOutput
    {
    public:
        TextureOutput() = default;
        void add_colour_output(tz::gl::TextureComponent* texture_component);
        void set_depth_output(tz::gl::TextureComponent* texture_component);
        const tz::gl::TextureComponent* get_first_colour_component() const;
        tz::gl::TextureComponent* get_first_colour_component();
        virtual RendererOutputType get_type() const final
        {
            return RendererOutputType::Texture;
        }
    private:
        std::vector<tz::gl::TextureComponent*> colour_outputs = {};
        tz::gl::TextureComponent* depth_attachment = nullptr;
    };
}

#endif // TOPAZ_GL_OUTPUT_HPP