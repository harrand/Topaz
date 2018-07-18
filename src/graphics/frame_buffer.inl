template<class Buffer, typename... Args>
Buffer& FrameBuffer::emplace(GLenum attachment, Args&&... args)
{
    if constexpr(std::is_same<Buffer, Texture>::value)
    {
        return this->emplace_texture(attachment, std::forward<Args>(args)...);
    }
    else if constexpr(std::is_same<Buffer, RenderBuffer>::value)
    {
        return this->emplace_renderbuffer(attachment, std::forward<Args>(args)...);
    }
    else
    {
        static_assert(std::is_void<Buffer>::value, "[Topaz Texture]: Texture::emplace has unsupported type.");
    }
}

template<typename... Args>
Texture& FrameBuffer::emplace_texture(GLenum attachment, Args&&... args)
{
    Texture& texture = std::get<Texture>((*(this->attachments.insert(std::make_pair(attachment, Texture(std::forward<Args>(args)...))).first)).second);
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture.texture_handle, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return texture;
}

template<typename... Args>
RenderBuffer& FrameBuffer::emplace_renderbuffer(GLenum attachment, Args&&... args)
{
    RenderBuffer& render_buffer = std::get<RenderBuffer>((*(this->attachments.insert(std::make_pair(attachment, RenderBuffer(std::forward<Args>(args)...))).first)).second);
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_handle);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, render_buffer.renderbuffer_handle);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return render_buffer;
}