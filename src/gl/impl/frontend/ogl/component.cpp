#if TZ_OGL
#include "gl/impl/frontend/ogl/component.hpp"

namespace tz::gl
{
    BufferComponentOGL::BufferComponentOGL(IResource* resource):
    buffer(ogl::Buffer::null()),
    resource(resource)
    {

    }

    const IResource* BufferComponentOGL::get_resource() const
    {
        return this->resource;
    }

    IResource* BufferComponentOGL::get_resource()
    {
        return this->resource;
    }

    const ogl::Buffer& BufferComponentOGL::get_buffer() const
    {
        return this->buffer;
    }

    ogl::Buffer& BufferComponentOGL::get_buffer()
    {
        return this->buffer;
    }

    void BufferComponentOGL::set_buffer(ogl::Buffer buffer)
    {
        this->buffer = std::move(buffer);
    }

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