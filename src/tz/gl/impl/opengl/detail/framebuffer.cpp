#if TZ_OGL
#include "tz/core/profile.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/framebuffer.hpp"

namespace tz::gl::ogl2
{
	framebuffer::framebuffer(framebuffer_info info):
	fb(0),
	info(info)
	{
		TZ_PROFZONE("OpenGL Backend - framebuffer Create", 0xFFAA0000);
		tz::assert(ogl2::is_initialised(), "Attempted to create framebuffer but ogl2 is not yet initialised. Please submit a bug report.");
		glCreateFramebuffers(1, &this->fb);

		// Deal with attachments.
		// First depth (if there is one).
		if(this->info.maybe_depth_attachment.has_value())
		{
			std::visit(
			[this](const auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, const image*>)
				{
					glNamedFramebufferTexture(this->fb, GL_DEPTH_ATTACHMENT, arg->native(), 0);
				}
				else if constexpr(std::is_same_v<T, const render_buffer*>)
				{
					glNamedFramebufferRenderbuffer(this->fb, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, arg->native());
				}
				else
				{
					tz::error("Unknown framebuffer_texture variant entry. Perhaps a new one has only been partially implemented? Please submit a bug report.");
				}
			}, this->info.maybe_depth_attachment.value());
		}

		std::vector<GLenum> draw_buffers;
		for(std::size_t i = 0; std::cmp_less(i, this->info.colour_attachments.length()); i++)
		{
			const GLenum attachment = GL_COLOR_ATTACHMENT0 + i;
			draw_buffers.push_back(attachment);
			std::visit(
			[this, attachment](const auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, const image*>)
				{
					glNamedFramebufferTexture(this->fb, attachment, arg->native(), 0);
				}
			}, this->info.colour_attachments[i]);
		}
		glNamedFramebufferDrawBuffers(this->fb, draw_buffers.size(), draw_buffers.data());

		tz::assert(glCheckNamedFramebufferStatus(this->fb, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Newly-created framebuffer was incomplete. Please submit a bug report.");
	}

	framebuffer::framebuffer(framebuffer&& move):
	fb(0),
	info()
	{
		*this = std::move(move);
	}

	framebuffer::~framebuffer()
	{
		glDeleteFramebuffers(1, &this->fb);
	}

	framebuffer& framebuffer::operator=(framebuffer&& rhs)
	{
		std::swap(this->fb, rhs.fb);
		std::swap(this->info, rhs.info);
		return *this;
	}
	
	bool framebuffer::has_depth_attachment() const
	{
		if(this->is_null())
		{
			return true;
		}
		return this->info.maybe_depth_attachment.has_value();
	}

	unsigned int framebuffer::colour_attachment_count() const
	{
		return this->info.colour_attachments.length();
	}

	tz::vec2ui framebuffer::get_dimensions() const
	{
		if(this->is_null())
		{
			return tz::window().get_dimensions();
		}
		return this->info.dimensions;
	}

	void framebuffer::bind() const
	{
		TZ_PROFZONE("OpenGL Backend - framebuffer Bind", 0xFFAA0000);
		glBindFramebuffer(GL_FRAMEBUFFER, this->fb);

		auto dims = this->get_dimensions();
		glViewport(0, 0, static_cast<GLsizei>(dims[0]), static_cast<GLsizei>(dims[1]));
	}

	void framebuffer::clear() const
	{
		TZ_PROFZONE("OpenGL Backend - framebuffer Clear", 0xFFAA0000);
		GLenum clear_flags = GL_COLOR_BUFFER_BIT;
		if(this->has_depth_attachment())
		{
			clear_flags |= GL_DEPTH_BUFFER_BIT;
		}
		glClear(clear_flags);
	}

	framebuffer framebuffer::null()
	{
		return {nullptr};
	}

	bool framebuffer::is_null() const
	{
		return this->fb == 0;
	}

	framebuffer::NativeType framebuffer::native() const
	{
		return this->fb;
	}

	framebuffer::framebuffer(std::nullptr_t):
	fb(0),
	info(){}
}

#endif // TZ_OGL
