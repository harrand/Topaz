#if TZ_OGL
#include "core/profiling/zone.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#include "gl/impl/backend/ogl2/framebuffer.hpp"

namespace tz::gl::ogl2
{
	Framebuffer::Framebuffer(FramebufferInfo info):
	framebuffer(0),
	info(info)
	{
		TZ_PROFZONE("OpenGL Backend - Framebuffer Create", TZ_PROFCOL_RED);
		tz_assert(ogl2::is_initialised(), "Attempted to create Framebuffer but ogl2 is not yet initialised. Please submit a bug report.");
		glCreateFramebuffers(1, &this->framebuffer);

		// Deal with attachments.
		// First depth (if there is one).
		if(this->info.maybe_depth_attachment.has_value())
		{
			std::visit(
			[this](const auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, const Image*>)
				{
					glNamedFramebufferTexture(this->framebuffer, GL_DEPTH_ATTACHMENT, arg->native(), 0);
				}
				else if constexpr(std::is_same_v<T, const Renderbuffer*>)
				{
					glNamedFramebufferRenderbuffer(this->framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, arg->native());
				}
				else
				{
					tz_error("Unknown FramebufferTexture variant entry. Perhaps a new one has only been partially implemented? Please submit a bug report.");
				}
			}, this->info.maybe_depth_attachment.value());
		}

		for(std::size_t i = 0; std::cmp_less(i, this->info.colour_attachments.length()); i++)
		{
			const GLenum attachment = GL_COLOR_ATTACHMENT0 + i;
			std::visit(
			[this, attachment](const auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, const Image*>)
				{
					glNamedFramebufferTexture(this->framebuffer, attachment, arg->native(), 0);
				}
			}, this->info.colour_attachments[i]);
		}

		tz_assert(glCheckNamedFramebufferStatus(this->framebuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Newly-created framebuffer was incomplete. Please submit a bug report.");
	}

	Framebuffer::Framebuffer(Framebuffer&& move):
	framebuffer(0),
	info()
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
		std::swap(this->info, rhs.info);
		return *this;
	}
	
	bool Framebuffer::has_depth_attachment() const
	{
		if(this->is_null())
		{
			return true;
		}
		return this->info.maybe_depth_attachment.has_value();
	}

	unsigned int Framebuffer::colour_attachment_count() const
	{
		return this->info.colour_attachments.length();
	}

	tz::Vec2ui Framebuffer::get_dimensions() const
	{
		if(this->is_null())
		{
			return {static_cast<unsigned int>(tz::window().get_width()), static_cast<unsigned int>(tz::window().get_height())};
		}
		return this->info.dimensions;
	}

	void Framebuffer::bind() const
	{
		TZ_PROFZONE("OpenGL Backend - Framebuffer Bind", TZ_PROFCOL_RED);
		glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
		auto dims = this->get_dimensions();
		glViewport(0, 0, static_cast<GLsizei>(dims[0]), static_cast<GLsizei>(dims[1]));
	}

	void Framebuffer::clear() const
	{
		TZ_PROFZONE("OpenGL Backend - Framebuffer Clear", TZ_PROFCOL_RED);
		GLenum clear_flags = GL_COLOR_BUFFER_BIT;
		if(this->has_depth_attachment())
		{
			clear_flags |= GL_DEPTH_BUFFER_BIT;
		}
		glClear(clear_flags);
	}

	Framebuffer Framebuffer::null()
	{
		return {nullptr};
	}

	bool Framebuffer::is_null() const
	{
		return this->framebuffer == 0;
	}

	Framebuffer::NativeType Framebuffer::native() const
	{
		return this->framebuffer;
	}

	Framebuffer::Framebuffer(std::nullptr_t):
	framebuffer(0),
	info(){}
}

#endif // TZ_OGL
