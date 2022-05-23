#include "gl/impl/backend/ogl2/sampler.hpp"
#if TZ_OGL
#include "gl/impl/frontend/ogl2/component.hpp"
#include "gl/impl/frontend/ogl2/convert.hpp"
#include "gl/resource.hpp"

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
		tz_assert(this->resource->get_access() == ResourceAccess::DynamicVariable, "Requested to resize a BufferComponentOGL, but the underlying resource was not ResourceAccess::DynamicVariable. Please submit a bug report.");
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
		return !this->resource->get_flags().contains(ResourceFlag::IndexBuffer);
	}

	ogl2::Buffer BufferComponentOGL::make_buffer() const
	{
		ogl2::BufferResidency residency;
		switch(this->resource->get_access())
		{
			default:
				tz_error("Unknown ResourceAccess. Please submit a bug report.");
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
		ogl2::BufferTarget tar;
		if(this->resource->get_flags().contains(ResourceFlag::IndexBuffer))
		{
			tar = ogl2::BufferTarget::Index;
		}
		else
		{
			tar = ogl2::BufferTarget::ShaderStorage;
		}
		return
		{{
			.target = tar,
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

	tz::Vec2ui ImageComponentOGL::get_dimensions() const
	{
		return this->image.get_dimensions();
	}

	ImageFormat ImageComponentOGL::get_format() const
	{
		return from_ogl2(this->image.get_format());
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
		tz_assert(this->resource->get_type() == ResourceType::Image, "ImageComponent was provided a resource which was not an ImageResource. Please submit a bug report.");
		const ImageResource* img_res = static_cast<const ImageResource*>(this->resource);
		return
		{{
			.format = to_ogl2(img_res->get_format()),
			.dimensions = img_res->get_dimensions(),
			.sampler = 
			{
				.min_filter = ogl2::LookupFilter::Nearest,
				.mag_filter = ogl2::LookupFilter::Nearest,
				.address_mode_s = ogl2::AddressMode::ClampToEdge,
				.address_mode_t = ogl2::AddressMode::ClampToEdge,
				.address_mode_r = ogl2::AddressMode::ClampToEdge
			}
		}};
	}
}

#endif // TZ_OGL
