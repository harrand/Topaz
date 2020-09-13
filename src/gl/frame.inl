#include <type_traits>

namespace tz::gl
{
    template<class TextureType, typename... Args>
		TextureType& Frame::emplace(GLenum attachment, Args&&... args)
        {
            if constexpr(std::is_same_v<TextureType, Texture>)
            {
                return this->emplace_texture(attachment, std::forward<Args>(args)...);
            }
            else if constexpr(std::is_same_v<TextureType, RenderBuffer>)
            {
                return this->emplace_renderbuffer(attachment, std::forward<Args>(args)...);
            }
            else
            {
                static_assert(std::is_void_v<TextureType>, "tz::gl::Frame::emplace<TextureType, Args...>(...): The TextureType is not supported.");
            }        
        }

		template<typename... Args>
		Texture& Frame::emplace_texture(GLenum attachment, Args&&... args)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, this->handle);
            auto& pair = this->attachments.emplace_back(attachment, tz::gl::Texture(std::forward<Args>(args)...));
            Texture& texture = std::get<Texture>(pair.second);
            this->pending_attachments.emplace(attachment, &texture);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return texture;
        }

		template<typename... Args>
		RenderBuffer& Frame::emplace_renderbuffer(GLenum attachment, Args&&... args)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, this->handle);
            auto& pair = this->attachments.emplace_back(attachment, tz::gl::RenderBuffer(std::forward<Args>(args)...));
            RenderBuffer& rb = std::get<RenderBuffer>(pair.second);
            rb.bind_to_frame(attachment);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return rb;
        }
}