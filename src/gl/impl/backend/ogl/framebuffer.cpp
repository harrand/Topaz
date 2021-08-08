#if TZ_OGL
#include "gl/impl/backend/ogl/framebuffer.hpp"

namespace tz::gl::ogl
{
    Framebuffer::Framebuffer():
    framebuffer(),
    attachments()
    {
        glCreateFramebuffers(1, &this->framebuffer);
    }

    Framebuffer::Framebuffer(Framebuffer&& move):
    framebuffer(0),
    attachments()
    {
        *this = std::move(move);
    }

    Framebuffer::~Framebuffer()
    {
        glDeleteFramebuffers(1, &this->framebuffer);
    }

    Framebuffer& Framebuffer::operator=(Framebuffer&& rhs)
    {
        std::swap(this->framebuffer, rhs.framebuffer);
        std::swap(this->attachments, rhs.attachments);
        return *this;
    }

    void Framebuffer::attach(GLenum attachment, const Texture& texture)
    {
        TextureVariant value = {&texture};
        this->attachments.emplace_back(attachment, value);

        glNamedFramebufferTexture(this->framebuffer, attachment, texture.native(), 0);
    }

    void Framebuffer::attach(GLenum attachment, const Renderbuffer& renderbuffer)
    {
        TextureVariant value = {&renderbuffer};
        this->attachments.emplace_back(attachment, value);

        glNamedFramebufferRenderbuffer(this->framebuffer, attachment, GL_RENDERBUFFER, renderbuffer.native());
    }

    void Framebuffer::set_output(GLenum attachment)
    {
        GLenum attachments[] = {attachment};
        glNamedFramebufferDrawBuffers(this->framebuffer, 1, attachments);
    }

    void Framebuffer::bind() const
    {
        tz_assert(this->complete(), "Framebuffer is not complete (OpenGL). This means attachments were somehow not set correctly.");
        glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
        // TODO: glViewport the correct dimensions.
    }

    bool Framebuffer::complete() const
    {
        GLenum status = glCheckNamedFramebufferStatus(this->framebuffer, GL_FRAMEBUFFER);
        return status == GL_FRAMEBUFFER_COMPLETE;
    }
}

#endif // TZ_OGL