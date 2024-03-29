#if TZ_OGL
#include "tz/gl/impl/opengl/detail/sampler.hpp"
#include "tz/gl/impl/opengl/component.hpp"
#include "tz/gl/impl/opengl/convert.hpp"
#include "tz/gl/resource.hpp"

namespace tz::gl
{
	buffer_component_ogl::buffer_component_ogl(iresource& resource):
	resource(&resource),
	buffer(this->make_buffer())
	{}

	const iresource* buffer_component_ogl::get_resource() const
	{
		return this->resource;
	}

	iresource* buffer_component_ogl::get_resource()
	{
		return this->resource;
	}

	std::size_t buffer_component_ogl::size() const
	{
		return this->buffer.size();
	}

	void buffer_component_ogl::resize(std::size_t sz)
	{
		ogl2::buffer& old_buffer = this->ogl_get_buffer();
		ogl2::buffer new_buffer = ogl2::buffer_helper::clone_resized(old_buffer, sz);
		// Just set the new buffer range, clone resized already sorts out the data for us.
		if(this->resource->get_access() == tz::gl::resource_access::dynamic_access)
		{
			this->resource->set_mapped_data(new_buffer.map_as<std::byte>());
		}
		else
		{
			auto old_data = this->resource->data();
			std::size_t copy_length = std::min(old_data.size_bytes(), sz);
			std::vector<std::byte> new_data;
			new_data.resize(sz);
			std::copy(old_data.begin(), old_data.begin() + copy_length, new_data.begin());
			this->resource->set_mapped_data(new_data);
		}
		std::swap(old_buffer, new_buffer);
	}

	ogl2::buffer& buffer_component_ogl::ogl_get_buffer()
	{
		return this->buffer;
	}

	bool buffer_component_ogl::ogl_is_descriptor_stakeholder() const
	{
		return !this->resource->get_flags().contains(resource_flag::index_buffer) && !this->resource->get_flags().contains(resource_flag::draw_indirect_buffer);
	}

	ogl2::buffer buffer_component_ogl::make_buffer() const
	{
		ogl2::buffer_residency residency;
		switch(this->resource->get_access())
		{
			default:
				tz::error("Unknown resource_access. Please submit a bug report.");
			[[fallthrough]];
			case resource_access::static_access:
				residency = ogl2::buffer_residency::static_fixed;
			break;
			case resource_access::dynamic_access:
				residency = ogl2::buffer_residency::dynamic;
			break;
		}
		return
		{{
			.target = ogl2::buffer_target::shader_storage,
			.residency = residency,
			.size_bytes = this->resource->data().size_bytes()
		}};
	}

	image_component_ogl::image_component_ogl(iresource& resource):
	resource(&resource),
	image(this->make_image())
	{}

	const iresource* image_component_ogl::get_resource() const
	{
		return this->resource;
	}

	iresource* image_component_ogl::get_resource()
	{
		return this->resource;
	}

	tz::vec2ui image_component_ogl::get_dimensions() const
	{
		return this->image.get_dimensions();
	}

	image_format image_component_ogl::get_format() const
	{
		return from_ogl2(this->image.get_format());
	}

	void image_component_ogl::resize(tz::vec2ui dims)
	{
		ogl2::image& old_image = this->ogl_get_image();
		ogl2::image new_image = ogl2::image_helper::clone_resized(old_image, dims);
		new_image.debug_set_name(old_image.debug_get_name());

		auto* ires = static_cast<image_resource*>(this->resource);
		ires->set_dimensions(dims);
		ires->resize_data(tz::gl::pixel_size_bytes(ires->get_format()) * dims[0] * dims[1]);
		std::swap(old_image, new_image);
	}

	const ogl2::image& image_component_ogl::ogl_get_image() const
	{
		return this->image;
	}

	ogl2::image& image_component_ogl::ogl_get_image()
	{
		return this->image;
	}

	ogl2::image image_component_ogl::make_image() const
	{
		tz::assert(this->resource->get_type() == resource_type::image, "image_component was provided a resource which was not an image_resource. Please submit a bug report.");
		const image_resource* img_res = static_cast<const image_resource*>(this->resource);
		ogl2::lookup_filter filter = ogl2::lookup_filter::nearest;
		ogl2::address_mode mode = ogl2::address_mode::clamp_to_edge;
#if TZ_DEBUG
		if(img_res->get_flags().contains({resource_flag::image_filter_nearest, resource_flag::image_filter_linear}))
		{
			tz::error("resource_flags included both image_filter_nearest and image_filter_linear, which are mutually exclusive. Please submit a bug report.");
		}
#endif
		if(img_res->get_flags().contains(resource_flag::image_filter_nearest))
		{
			filter = ogl2::lookup_filter::nearest;
		}
		if(img_res->get_flags().contains(resource_flag::image_filter_linear))
		{
			filter = ogl2::lookup_filter::linear;
		}

		if(img_res->get_flags().contains({resource_flag::image_wrap_clamp_edge, resource_flag::image_wrap_repeat, resource_flag::image_wrap_mirrored_repeat}))
		{
			tz::error("resource_flags included all 3 of image_wrap_clamp_edge, image_wrap_repeat and image_wrap_mirrored_repeat, all of which are mutually exclusive. Please submit a bug report.");
		}
		if(img_res->get_flags().contains(resource_flag::image_wrap_clamp_edge))
		{
			mode = ogl2::address_mode::clamp_to_edge;
		}
		if(img_res->get_flags().contains(resource_flag::image_wrap_repeat))
		{
			mode = ogl2::address_mode::repeat;
		}
		if(img_res->get_flags().contains(resource_flag::image_wrap_mirrored_repeat))
		{
			mode = ogl2::address_mode::mirrored_repeat;
		}
		return
		{{
			.format = to_ogl2(img_res->get_format()),
			.dimensions = img_res->get_dimensions(),
			.shader_sampler = 
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
