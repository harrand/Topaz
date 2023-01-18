#ifndef TOPAZ_GL2_IMPL_FRONTEND_OGL2_COMPONENT_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_OGL2_COMPONENT_HPP
#if TZ_OGL
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/api/component.hpp"
#include "tz/gl/impl/opengl/detail/buffer.hpp"
#include "tz/gl/impl/opengl/detail/image.hpp"

namespace tz::gl
{
	using namespace tz::gl;
	class BufferComponentOGL : public icomponent
	{
	public:
		BufferComponentOGL(IResource& resource);
		virtual const IResource* get_resource() const final;
		virtual IResource* get_resource() final;
		// Satisfy buffer_component_type
		std::size_t size() const;
		void resize(std::size_t sz);

		ogl2::Buffer& ogl_get_buffer();
		bool ogl_is_descriptor_stakeholder() const;
	private:
		ogl2::Buffer make_buffer() const;

		IResource* resource;
		ogl2::Buffer buffer;
	};
	static_assert(buffer_component_type<BufferComponentOGL>);

	class ImageComponentOGL : public icomponent
	{
	public:
		ImageComponentOGL(IResource& resource);
		virtual const IResource* get_resource() const final;
		virtual IResource* get_resource() final;
		// Satisfy image_component_type
		hdk::vec2ui get_dimensions() const;
		image_format get_format() const;
		void resize(hdk::vec2ui dims);

		const ogl2::Image& ogl_get_image() const;
		ogl2::Image& ogl_get_image();
	private:
		ogl2::Image make_image() const;
		
		IResource* resource;
		ogl2::Image image;
	};
	static_assert(image_component_type<ImageComponentOGL>);
}

#endif // TZ_OGL
#endif // TOPAZ_GL2_IMPL_FRONTEND_OGL2_COMPONENT_HPP
