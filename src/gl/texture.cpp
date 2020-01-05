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
        this->internal_bind();
        this->internal_unbind();
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

    void Texture::resize(const TextureDataDescriptor& descriptor)
    {
        this->descriptor = descriptor;
        this->internal_bind();
        const auto& desc = this->descriptor.value();
        glTexImage2D(GL_TEXTURE_2D, 0, desc.internal_format, static_cast<GLsizei>(desc.width), static_cast<GLsizei>(desc.height), 0, desc.format, desc.component_type, nullptr);
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

    void Texture::bind(std::size_t binding_id) const
    {
        glActiveTexture(GL_TEXTURE0 + binding_id);
        this->internal_bind();
    }

    void Texture::bind_to_frame(GLenum attachment) const
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, this->handle, 0);
    }

    void Texture::internal_bind() const
    {
        glBindTexture(GL_TEXTURE_2D, this->handle);
    }

    void Texture::internal_unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    RenderBuffer::RenderBuffer(): handle(0)
    {
        glCreateRenderbuffers(1, &this->handle);
        this->bind();
        this->unbind();
    }

    RenderBuffer::RenderBuffer(TextureDataDescriptor descriptor): RenderBuffer()
    {
        this->descriptor = descriptor;
        this->bind();
        glRenderbufferStorage(GL_RENDERBUFFER, static_cast<GLenum>(this->descriptor.internal_format), static_cast<GLsizei>(this->descriptor.width), static_cast<GLsizei>(this->descriptor.height));
        this->unbind();
    }

    RenderBuffer::RenderBuffer(RenderBuffer&& move): handle(move.handle), descriptor(move.descriptor)
    {
        move.handle = 0;
    }

    RenderBuffer::~RenderBuffer()
    {
        glDeleteRenderbuffers(1, &this->handle);
    }

    void RenderBuffer::bind() const
    {
        glBindRenderbuffer(GL_RENDERBUFFER, this->handle);
    }

    void RenderBuffer::unbind() const
    {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    void RenderBuffer::bind_to_frame(GLenum attachment) const
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, this->handle);
    }
}