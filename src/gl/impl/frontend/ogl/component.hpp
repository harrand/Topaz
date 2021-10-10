#ifndef TOPAZ_GL_IMPL_FRONTEND_OGL_COMPONENT_HPP
#define TOPAZ_GL_IMPL_FRONTEND_OGL_COMPONENT_HPP
#if TZ_OGL
#include "gl/api/component.hpp"

#include "gl/impl/backend/ogl/texture.hpp"
#include "gl/impl/backend/ogl/buffer.hpp"

namespace tz::gl
{
	class BufferComponentOGL : public IComponent
	{
	public:
		BufferComponentOGL(IResource* resource);
		BufferComponentOGL() = default;
		virtual const IResource* get_resource() const final;
		virtual IResource* get_resource() final;
		const ogl::Buffer& get_buffer() const;
		ogl::Buffer& get_buffer();
		void set_buffer(ogl::Buffer buffer);
	private:
		ogl::Buffer buffer = ogl::Buffer::null();
		IResource* resource = nullptr;
	};

	class TextureComponentOGL : public IComponent
	{
	public:
		TextureComponentOGL(IResource* resource, ogl::Texture texture);
		TextureComponentOGL(ogl::Texture texture);
		void clear_and_resize(unsigned int width, unsigned int height);
		virtual const IResource* get_resource() const override;
		virtual IResource* get_resource() override;

		const ogl::Texture& get_texture() const;
		ogl::Texture& get_texture();
	private:
		IResource* resource;
		ogl::Texture texture;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_FRONTEND_OGL_COMPONENT_HPP