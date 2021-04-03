#include "gl/texture.hpp"
#include "dui/window.hpp"

namespace tz::gl
{
	static TextureSentinel global_sentinel;

	TextureSentinel& sentinel()
	{
		return global_sentinel;
	}

	bool dui_draw_bindless(BindlessTextureHandle handle, tz::Vec2 size)
	{
		auto opt_tex_name = sentinel().get_texture_name(handle);
		if(!opt_tex_name.has_value()) [[unlikely]]
		{
			return false;
		}

		auto Vec2ToImVec2 = [](tz::Vec2 v)->ImVec2{return {v[0], v[1]};};
		auto Vec4ToImVec4 = [](tz::Vec4 v)->ImVec4{return {v[0], v[1], v[2], v[3]};};
		sentinel().notify_usage(handle);
		ImGui::Image(opt_tex_name.value(), Vec2ToImVec2(size), Vec2ToImVec2(tz::Vec2{0.0f, 0.0f}), Vec2ToImVec2(tz::Vec2{1.0f, 1.0f}), Vec4ToImVec4(tz::Vec4{1.0f, 1.0f, 1.0f, 1.0f}), Vec4ToImVec4(tz::Vec4{0.0f, 0.0f, 0.0f, 0.0f}));
		return true;
	}

	bool TextureDataDescriptor::operator==(const TextureDataDescriptor& rhs) const
	{
		return this->component_type == rhs.component_type && this->internal_format == rhs.internal_format && this->format == rhs.format;
	}

	bool TextureDataDescriptor::operator!=(const TextureDataDescriptor& rhs) const
	{
		return this->component_type != rhs.component_type || this->internal_format != rhs.internal_format || this->format != rhs.format;
	}

	Texture::Texture(): handle(0), descriptor(std::nullopt), bindless(std::nullopt)
	{
		glGenTextures(1, &this->handle);
		this->internal_bind();
		this->internal_unbind();
	}

	Texture::Texture(Texture&& move): handle(move.handle), descriptor(move.descriptor), bindless(move.bindless)
	{
		topaz_assert(move.handle != 0, "tz::gl::Texture::Texture(Texture&&): Provided move candidate was invalid (move.handle == ", move.handle, ")");
		move.handle = 0;
	}

	Texture& Texture::operator=(Texture&& rhs)
	{
		std::swap(this->handle, rhs.handle);
		auto desc_tmp = this->descriptor;
		this->descriptor = rhs.descriptor;
		rhs.descriptor = desc_tmp;
		return *this;
	}

	Texture::~Texture()
	{
		// "glDeleteTextures silently ignores 0's and names that do not correspond to existing textures." - https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glDeleteTextures.xml
		glDeleteTextures(1, &this->handle);
	}

	void Texture::set_parameters(const TextureParameters& params)
	{
		topaz_hard_assert(!this->is_terminal(), "Cannot set parameters on a terminal texture. Do this before making the texture terminal -- Otherwise this could crash the GPU/entire OS.");
		this->internal_bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(params.min_filter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(params.mag_filter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(params.horizontal_wrap));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(params.vertical_wrap));
	}

	void Texture::resize(const TextureDataDescriptor& descriptor)
	{
		topaz_hard_assert(!this->is_terminal(), "tz::gl::Texture::resize(...): Must never resize a terminal texture! Doing so could crash the GPU/entire OS.");
		this->descriptor = descriptor;
		this->internal_bind();
		const auto& desc = this->descriptor.value();
		glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(desc.internal_format), static_cast<GLsizei>(desc.width), static_cast<GLsizei>(desc.height), 0, static_cast<GLenum>(desc.format), static_cast<GLenum>(desc.component_type), nullptr);
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

	bool Texture::is_terminal() const
	{
		return this->bindless.has_value();
	}

	void Texture::make_terminal()
	{
		topaz_assert(!this->is_terminal(), "tz::gl::Texture::make_terminal(): Texture is already terminal!");
		this->bindless = glGetTextureHandleARB(this->handle);
		tz::gl::sentinel().register_handle(this->handle, this->bindless.value());
		glMakeTextureHandleResidentARB(this->bindless.value());
		tz::gl::sentinel().make_resident(this->bindless.value());
	}

	BindlessTextureHandle Texture::get_terminal_handle() const
	{
		topaz_hard_assert(this->is_terminal(), "tz::gl::Texture::get_terminal_handle(): Texture is not terminal!");
		topaz_hard_assert(this->bindless.has_value(), "tz::gl::Texture::get_terminal_handle(): Bindless handle isn't set.");
		auto handle = this->bindless.value();
		tz::gl::sentinel().notify_usage(handle);
		return handle;
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

	void Texture::dui_draw(tz::Vec2 size, tz::Vec2 uv0, tz::Vec2 uv1, tz::Vec4 tint_col, tz::Vec4 border_col) const
	{
		auto Vec2ToImVec2 = [](tz::Vec2 v)->ImVec2{return {v[0], v[1]};};
		auto Vec4ToImVec4 = [](tz::Vec4 v)->ImVec4{return {v[0], v[1], v[2], v[3]};};
		if(size == tz::Vec2{-1.0f, -1.0f})
		{
			size = tz::Vec2{static_cast<float>(this->get_width()), static_cast<float>(this->get_height())};
		}
		ImGui::Image(this->handle, Vec2ToImVec2(size), Vec2ToImVec2(uv0), Vec2ToImVec2(uv1), Vec4ToImVec4(tint_col), Vec4ToImVec4(border_col));
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

	void RenderBuffer::bind_to_frame(FrameAttachment attachment) const
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, static_cast<GLenum>(attachment), GL_RENDERBUFFER, this->handle);
	}
}