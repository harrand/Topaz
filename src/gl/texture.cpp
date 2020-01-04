#include "gl/texture.hpp"

namespace tz::gl
{
    bool TextureDataDescriptor::operator==(const TextureDataDescriptor& rhs) const
    {
        return this->component_type == rhs.component_type && this->internal_format == rhs.internal_format && this->format == rhs.format;
    }

    bool TextureDataDescriptor::operator!=(const TextureDataDescriptor& rhs) const
    {
        return this->component_type != rhs.component_type || this->internal_format != rhs.internal_format || this->format != rhs.format;
    }

    Texture::Texture(): handle(0), descriptor(std::nullopt)
    {
        glGenTextures(1, &this->handle);
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &this->handle);
    }

    void Texture::set_parameters(const TextureParameters& params)
    {
        this->internal_bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(params.min_filter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(params.mag_filter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(params.horizontal_wrap));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(params.vertical_wrap));
    }

    std::size_t Texture::size() const
    {
        return this->get_width() * this->get_height();
    }

    bool Texture::empty() const
    {
        return this->size() == 0;
    }

    unsigned int Texture::get_width() const
    {
        if(this->descriptor.has_value())
            return this->descriptor.value().width;
        return 0;
    }

    unsigned int Texture::get_height() const
    {
        if(this->descriptor.has_value())
            return this->descriptor.value().height;
        return 0;
    }

    void Texture::internal_bind() const
    {
        glBindTexture(GL_TEXTURE_2D, this->handle);
    }

    void Texture::internal_unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}