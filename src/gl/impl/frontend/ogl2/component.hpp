#ifndef TOPAZ_GL2_IMPL_FRONTEND_OGL2_COMPONENT_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_OGL2_COMPONENT_HPP
#if TZ_OGL
#include "gl/declare/image_format.hpp"
#include "gl/api/component.hpp"
#include "gl/impl/backend/ogl2/buffer.hpp"
#include "gl/impl/backend/ogl2/image.hpp"

namespace tz::gl
{
	using namespace tz::gl;
	class BufferComponentOGL : public IComponent
	{
	public:
		BufferComponentOGL(IResource& resource);
		virtual const IResource* get_resource() const final;
		virtual IResource* get_resource() final;
		// Satisfy BufferComponentType
		std::size_t size() const;
		void resize(std::size_t sz);

		ogl2::Buffer& ogl_get_buffer();
		bool ogl_is_descriptor_stakeholder() const;
	private:
		ogl2::Buffer make_buffer() const;

		IResource* resource;
		ogl2::Buffer buffer;
	};
	static_assert(BufferComponentType<BufferComponentOGL>);

	class ImageComponentOGL : public IComponent
	{
	public:
		ImageComponentOGL(IResource& resource);
		virtual const IResource* get_resource() const final;
		virtual IResource* get_resource() final;
		// Satisfy ImageComponentType
		tz::Vec2ui get_dimensions() const;
		ImageFormat get_format() const;
		void resize(tz::Vec2ui dims);

		const ogl2::Image& ogl_get_image() const;
		ogl2::Image& ogl_get_image();
	private:
		ogl2::Image make_image() const;
		
		IResource* resource;
		ogl2::Image image;
	};
	static_assert(ImageComponentType<ImageComponentOGL>);
}

#endif // TZ_OGL
#endif // TOPAZ_GL2_IMPL_FRONTEND_OGL2_COMPONENT_HPP
