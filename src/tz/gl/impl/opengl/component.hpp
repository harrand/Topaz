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
	class buffer_component_ogl : public icomponent
	{
	public:
		buffer_component_ogl(iresource& resource);
		virtual const iresource* get_resource() const final;
		virtual iresource* get_resource() final;
		// Satisfy buffer_component_type
		std::size_t size() const;
		void resize(std::size_t sz);

		ogl2::buffer& ogl_get_buffer();
		bool ogl_is_descriptor_stakeholder() const;
	private:
		ogl2::buffer make_buffer() const;

		iresource* resource;
		ogl2::buffer buffer;
	};
	static_assert(buffer_component_type<buffer_component_ogl>);

	class image_component_ogl : public icomponent
	{
	public:
		image_component_ogl(iresource& resource);
		virtual const iresource* get_resource() const final;
		virtual iresource* get_resource() final;
		// Satisfy image_component_type
		tz::vec2ui get_dimensions() const;
		image_format get_format() const;
		void resize(tz::vec2ui dims);

		const ogl2::image& ogl_get_image() const;
		ogl2::image& ogl_get_image();
	private:
		ogl2::image make_image() const;
		
		iresource* resource;
		ogl2::image image;
	};
	static_assert(image_component_type<image_component_ogl>);
}

#endif // TZ_OGL
#endif // TOPAZ_GL2_IMPL_FRONTEND_OGL2_COMPONENT_HPP
