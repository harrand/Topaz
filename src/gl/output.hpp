#ifndef TOPAZ_GL_OUTPUT_HPP
#define TOPAZ_GL_OUTPUT_HPP
#include "gl/api/renderer.hpp"

namespace tz::gl
{
    class TextureOutput : public IRendererOutput
    {
    public:
        TextureOutput(unsigned int width, unsigned int height);
        virtual RendererOutputType get_type() const final
        {
            return RendererOutputType::Texture;
        }
    private:
        unsigned int width;
        unsigned int height;
    };
}

#endif // TOPAZ_GL_OUTPUT_HPP