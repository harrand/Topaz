#if TZ_OGL
#include "tz/gl/impl/opengl/detail/sampler.hpp"
#include "tz/gl/impl/opengl/component.hpp"
#include "tz/gl/impl/opengl/convert.hpp"
#include "tz/gl/resource.hpp"

namespace tz::gl
{
	BufferComponentOGL::BufferComponentOGL(iresource& resource):
	resource(&resource),
	buffer(this->make_buffer())
	{}

	const iresource* BufferComponentOGL::get_resource() const
	{
		return this->resource;
	}

	iresource* BufferComponentOGL::get_resource()
	{
		return this->resource;
	}

	std::size_t BufferComponentOGL::size() const
	{
		return this->buffer.size();
	}

	void BufferComponentOGL::resize(std::size_t sz)
	{
		hdk::assert(this->resource->get_access() == resource_access::dynamic_variable, "Requested to resize a BufferComponentOGL, but the underlying resource was not resource_access::dynamic_variable. Please submit a bug report.");
		ogl2::Buffer& old_buffer = this->ogl_get_buffer();
		ogl2::Buffer new_buffer = ogl2::buffer::clone_resized(old_buffer, sz);
		// Just set the new buffer range, clone resized already sorts out the data for us.
		this->resource->set_mapped_data(new_buffer.map_as<std::byte>());
		std::swap(old_buffer, new_buffer);
	}

	ogl2::Buffer& BufferComponentOGL::ogl_get_buffer()
	{
		return this->buffer;
	}

	bool BufferComponentOGL::ogl_is_descriptor_stakeholder() const
	{
		return !this->resource->get_flags().contains(resource_flag::index_buffer) && !this->resource->get_flags().contains(resource_flag::draw_indirect_buffer);
	}

	ogl2::Buffer BufferComponentOGL::make_buffer() const
	{
		ogl2::BufferResidency residency;
		switch(this->resource->get_access())
		{
			default:
				hdk::error("Unknown resource_access. Please submit a bug report.");
			[[fallthrough]];
			case resource_access::static_fixed:
				residency = ogl2::BufferResidency::Static;
			break;
			case resource_access::dynamic_fixed:
			[[fallthrough]];
			case resource_access::dynamic_variable:
				residency = ogl2::BufferResidency::Dynamic;
			break;
		}
		return
		{{
			.target = ogl2::BufferTarget::ShaderStorage,
			.residency = residency,
			.size_bytes = this->resource->data().size_bytes()
		}};
	}

	ImageComponentOGL::ImageComponentOGL(iresource& resource):
	resource(&resource),
	image(this->make_image())
	{}

	const iresource* ImageComponentOGL::get_resource() const
	{
		return this->resource;
	}

	iresource* ImageComponentOGL::get_resource()
	{
		return this->resource;
	}

	hdk::vec2ui ImageComponentOGL::get_dimensions() const
	{
		return this->image.get_dimensions();
	}

	image_format ImageComponentOGL::get_format() const
	{
		return from_ogl2(this->image.get_format());
	}

	void ImageComponentOGL::resize(hdk::vec2ui dims)
	{
		hdk::assert(this->resource->get_access() == resource_access::dynamic_variable, "Requested resize of ImageComponentOGL, but the underlying resource did not have resource_access::dynamic_variable. Please submit a bug report.");
		ogl2::Image& old_image = this->ogl_get_image();
		ogl2::Image new_image = ogl2::image::clone_resized(old_image, dims);

		auto* ires = static_cast<ImageResource*>(this->resource);
		ires->set_dimensions(dims);
		ires->resize_data(tz::gl::pixel_size_bytes(ires->get_format()) * dims[0] * dims[1]);
		std::swap(old_image, new_image);
	}

	const ogl2::Image& ImageComponentOGL::ogl_get_image() const
	{
		return this->image;
	}

	ogl2::Image& ImageComponentOGL::ogl_get_image()
	{
		return this->image;
	}

	ogl2::Image ImageComponentOGL::make_image() const
	{
		hdk::assert(this->resource->get_type() == resource_type::image, "ImageComponent was provided a resource which was not an ImageResource. Please submit a bug report.");
		const ImageResource* img_res = static_cast<const ImageResource*>(this->resource);
		ogl2::LookupFilter filter = ogl2::LookupFilter::Nearest;
		ogl2::AddressMode mode = ogl2::AddressMode::ClampToEdge;
#if HDK_DEBUG
		if(img_res->get_flags().contains({resource_flag::image_filter_nearest, resource_flag::image_filter_linear}))
		{
			hdk::error("resource_flags included both image_filter_nearest and image_filter_linear, which are mutually exclusive. Please submit a bug report.");
		}
#endif
		if(img_res->get_flags().contains(resource_flag::image_filter_nearest))
		{
			filter = ogl2::LookupFilter::Nearest;
		}
		if(img_res->get_flags().contains(resource_flag::image_filter_linear))
		{
			filter = ogl2::LookupFilter::Linear;
		}

		if(img_res->get_flags().contains({resource_flag::image_wrap_clamp_edge, resource_flag::image_wrap_repeat, resource_flag::image_wrap_mirrored_repeat}))
		{
			hdk::error("resource_flags included all 3 of image_wrap_clamp_edge, image_wrap_repeat and image_wrap_mirrored_repeat, all of which are mutually exclusive. Please submit a bug report.");
		}
		if(img_res->get_flags().contains(resource_flag::image_wrap_clamp_edge))
		{
			mode = ogl2::AddressMode::ClampToEdge;
		}
		if(img_res->get_flags().contains(resource_flag::image_wrap_repeat))
		{
			mode = ogl2::AddressMode::Repeat;
		}
		if(img_res->get_flags().contains(resource_flag::image_wrap_mirrored_repeat))
		{
			mode = ogl2::AddressMode::MirroredRepeat;
		}
		return
		{{
			.format = to_ogl2(img_res->get_format()),
			.dimensions = img_res->get_dimensions(),
			.sampler = 
			{
				.min_filter = filter,
				.mag_filter = filter,
				.address_mode_s = mode,
				.address_mode_t = mode,
				.address_mode_r = mode
			}
		}};
	}
}

#endif // TZ_OGL
