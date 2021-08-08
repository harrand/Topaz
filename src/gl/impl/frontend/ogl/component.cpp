#if TZ_OGL
#include "gl/impl/frontend/ogl/component.hpp"

namespace tz::gl
{
    TextureComponentOGL::TextureComponentOGL(IResource* resource, ogl::Texture texture):
    resource(resource),
    texture(std::move(texture))
    {}

    TextureComponentOGL::TextureComponentOGL(ogl::Texture texture):
    TextureComponentOGL(nullptr, std::move(texture))
    {}

    void TextureComponentOGL::clear_and_resize(unsigned int width, unsigned int height)
    {
        this->texture.resize_and_clear(width, height);
    }

    const IResource* TextureComponentOGL::get_resource() const
    {
        return this->resource;
    }

    IResource* TextureComponentOGL::get_resource()
    {
        return this->resource;
    }

    const ogl::Texture& TextureComponentOGL::get_texture() const
    {
        return this->texture;
    }

    ogl::Texture& TextureComponentOGL::get_texture()
    {
        return this->texture;
    }
}

#endif // TZ_OGL