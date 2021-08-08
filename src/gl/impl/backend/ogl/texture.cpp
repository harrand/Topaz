#if TZ_OGL
#include "gl/impl/backend/ogl/texture.hpp"

namespace tz::gl::ogl
{
    Texture::Texture(unsigned int width, unsigned int height, Format format, TextureParameters parameters):
    texture(0),
    width(width),
    height(height),
    format(format),
    parameters(parameters)
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &this->texture);

        glTextureParameteri(this->texture, GL_TEXTURE_WRAP_S, this->parameters.tex_wrap_s);
        glTextureParameteri(this->texture, GL_TEXTURE_WRAP_T, this->parameters.tex_wrap_t);
        glTextureParameteri(this->texture, GL_TEXTURE_WRAP_R, this->parameters.tex_wrap_u);

        glTextureParameteri(this->texture, GL_TEXTURE_MIN_FILTER, this->parameters.min_filter);
        glTextureParameteri(this->texture, GL_TEXTURE_MAG_FILTER, this->parameters.mag_filter);

        glBindTexture(GL_TEXTURE_2D, this->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, this->format.internal_format, this->width, this->height, 0, this->format.format, this->format.type, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture::Texture(Texture&& move):
    texture(0),
    width(),
    height(),
    format(),
    parameters()
    {
        *this = std::move(move);
    }

    Texture& Texture::operator=(Texture&& rhs)
    {
        std::swap(this->texture, rhs.texture);
        std::swap(this->width, rhs.width);
        std::swap(this->height, rhs.height);
        std::swap(this->format, rhs.format);
        std::swap(this->parameters, rhs.parameters);
    }
}

#endif // TZ_OGL