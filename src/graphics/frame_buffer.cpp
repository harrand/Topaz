#include "frame_buffer.hpp"

RenderBuffer::RenderBuffer(int width, int height, GLenum internal_format): width(width), height(height), internal_format(internal_format), renderbuffer_handle(0)
{
    glGenRenderbuffers(1, &(this->renderbuffer_handle));
    glBindRenderbuffer(GL_RENDERBUFFER, this->renderbuffer_handle);
    glRenderbufferStorage(GL_RENDERBUFFER, this->internal_format, this->width, this->height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::RenderBuffer(RenderBuffer&& move): width(move.width), height(move.height), internal_format(move.internal_format), renderbuffer_handle(move.renderbuffer_handle)
{
    move.renderbuffer_handle = 0;
}

RenderBuffer::~RenderBuffer()
{
    // still silently ignores 0's.
    glDeleteRenderbuffers(1, &(this->renderbuffer_handle));
}

FrameBuffer::FrameBuffer(int width, int height): width(width), height(height), framebuffer_handle(0), attachments({})
{
    glGenFramebuffers(1, &(this->framebuffer_handle));
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &(this->framebuffer_handle));
}

const std::unordered_map<GLenum, std::variant<Texture, RenderBuffer>>& FrameBuffer::get_attachments() const
{
    return this->attachments;
}

std::unordered_map<GLenum, std::reference_wrapper<const Texture>> FrameBuffer::get_texture_attachments() const
{
    std::unordered_map<GLenum, std::reference_wrapper<const Texture>> result;
    for(const auto& [attachment_type, variant] : this->attachments)
    {
        auto texture = std::get_if<Texture>(&variant);
        if(texture == nullptr)
            continue;
        result.emplace(attachment_type, std::cref(*texture));
    }
    return result;
}

bool FrameBuffer::valid() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_handle);
    bool valid = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return valid;
}

bool FrameBuffer::has_colour(unsigned int attachment_index) const
{
    if(attachment_index > tz::graphics::maximum_framebuffer_attachments)
    {
        std::cerr << "FrameBuffer attachment_index query '" << attachment_index << "' does not harbour a valid GL_COLOR_ATTACHMENT. Implementation-defined hardware maximum limit is attachment" << tz::graphics::maximum_framebuffer_attachments << "or below.\n";
        return false;
    }
    return this->get_attachments().find(GL_COLOR_ATTACHMENT0 + attachment_index) != this->get_attachments().cend();
}

bool FrameBuffer::has_depth() const
{
    return this->get_attachments().find(GL_DEPTH_ATTACHMENT) != this->get_attachments().cend();
}

bool FrameBuffer::has_stencil() const
{
    return this->get_attachments().find(GL_STENCIL_ATTACHMENT) != this->get_attachments().cend();
}

void FrameBuffer::set_output_attachment(GLenum attachment) const
{
    if(this->attachments.find(attachment) == this->attachments.cend() && attachment != GL_NONE)
    {
        std::cerr << "FrameBuffer render attachment type has no corresponding attachment; setting to default (which is GL_COLOR_ATTACHMENT0).\n";
        attachment = GL_COLOR_ATTACHMENT0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_handle);
    glDrawBuffer(attachment);
    glReadBuffer(attachment);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::clear(GLbitfield mask, float r, float g, float b, float a) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_handle);
    glClearColor(r, g, b, a);
    glClear(mask);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::set_render_target() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_handle);
    glViewport(0, 0, this->width, this->height);
}

ShadowMap::ShadowMap(int width, int height): FrameBuffer(width, height), depth_texture(this->emplace_texture(GL_DEPTH_ATTACHMENT, width, height, tz::graphics::TextureComponent::DEPTH_TEXTURE))
{
    this->set_output_attachment(GL_NONE);
}

const Texture& ShadowMap::get_depth_texture() const
{
    return this->depth_texture;
}