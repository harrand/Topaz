#if TZ_OGL
#include "tz/gl/impl/opengl/detail/sampler.hpp"
#include "tz/gl/impl/opengl/component.hpp"
#include "tz/gl/impl/opengl/convert.hpp"
#include "tz/gl/resource.hpp"

namespace tz::gl
{
	BufferComponentOGL::BufferComponentOGL(IResource& resource):
	resource(&resource),
	buffer(this->make_buffer())
	{}

	const IResource* BufferComponentOGL::get_resource() const
	{
		return this->resource;
	}

	IResource* BufferComponentOGL::get_resource()
	{
		return this->resource;
	}

	std::size_t BufferComponentOGL::size() const
	{
		return this->buffer.size();
	}

	void BufferComponentOGL::resize(std::size_t sz)
	{
		hdk::assert(this->resource->get_access() == ResourceAccess::DynamicVariable, "Requested to resize a BufferComponentOGL, but the underlying resource was not ResourceAccess::DynamicVariable. Please submit a bug report.");
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
		return !this->resource->get_flags().contains(ResourceFlag::IndexBuffer) && !this->resource->get_flags().contains(ResourceFlag::DrawIndirectBuffer);
	}

	ogl2::Buffer BufferComponentOGL::make_buffer() const
	{
		ogl2::BufferResidency residency;
		switch(this->resource->get_access())
		{
			default:
				hdk::error("Unknown ResourceAccess. Please submit a bug report.");
			[[fallthrough]];
			case ResourceAccess::StaticFixed:
				residency = ogl2::BufferResidency::Static;
			break;
			case ResourceAccess::DynamicFixed:
			[[fallthrough]];
			case ResourceAccess::DynamicVariable:
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

	ImageComponentOGL::ImageComponentOGL(IResource& resource):
	resource(&resource),
	image(this->make_image())
	{}

	const IResource* ImageComponentOGL::get_resource() const
	{
		return this->resource;
	}

	IResource* ImageComponentOGL::get_resource()
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
		hdk::assert(this->resource->get_access() == ResourceAccess::DynamicVariable, "Requested resize of ImageComponentOGL, but the underlying resource did not have ResourceAccess::DynamicVariable. Please submit a bug report.");
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
		hdk::assert(this->resource->get_type() == ResourceType::Image, "ImageComponent was provided a resource which was not an ImageResource. Please submit a bug report.");
		const ImageResource* img_res = static_cast<const ImageResource*>(this->resource);
		ogl2::LookupFilter filter = ogl2::LookupFilter::Nearest;
		ogl2::AddressMode mode = ogl2::AddressMode::ClampToEdge;
#if HDK_DEBUG
		if(img_res->get_flags().contains({ResourceFlag::ImageFilterNearest, ResourceFlag::ImageFilterLinear}))
		{
			hdk::error("ResourceFlags included both ImageFilterNearest and ImageFilterLinear, which are mutually exclusive. Please submit a bug report.");
		}
#endif
		if(img_res->get_flags().contains(ResourceFlag::ImageFilterNearest))
		{
			filter = ogl2::LookupFilter::Nearest;
		}
		if(img_res->get_flags().contains(ResourceFlag::ImageFilterLinear))
		{
			filter = ogl2::LookupFilter::Linear;
		}

		if(img_res->get_flags().contains({ResourceFlag::ImageWrapClampEdge, ResourceFlag::ImageWrapRepeat, ResourceFlag::ImageWrapMirroredRepeat}))
		{
			hdk::error("ResourceFlags included all 3 of ImageWrapClampEdge, ImageWrapRepeat and ImageWrapMirroredRepeat, all of which are mutually exclusive. Please submit a bug report.");
		}
		if(img_res->get_flags().contains(ResourceFlag::ImageWrapClampEdge))
		{
			mode = ogl2::AddressMode::ClampToEdge;
		}
		if(img_res->get_flags().contains(ResourceFlag::ImageWrapRepeat))
		{
			mode = ogl2::AddressMode::Repeat;
		}
		if(img_res->get_flags().contains(ResourceFlag::ImageWrapMirroredRepeat))
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
