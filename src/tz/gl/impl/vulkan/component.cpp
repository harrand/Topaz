#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/image_format.hpp"
#include "tz/gl/impl/vulkan/component.hpp"
#include "tz/gl/impl/vulkan/convert.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/device.hpp"
#include "tz/core/debug.hpp"

namespace tz::gl
{
	using namespace tz::gl;
	buffer_component_vulkan::buffer_component_vulkan(iresource& resource):
	resource(&resource),
	buffer(this->make_buffer())
	{}

	const iresource* buffer_component_vulkan::get_resource() const
	{
		return this->resource;
	}

	iresource* buffer_component_vulkan::get_resource()
	{
		return this->resource;
	}

	std::size_t buffer_component_vulkan::size() const
	{
		tz::assert(this->resource->data().size_bytes() == this->buffer.size(), "buffer_component Size does not match its iresource data size. Please submit a bug report.");
		return this->buffer.size();
	}

	void buffer_component_vulkan::resize(std::size_t sz)
	{
		tz::assert(this->resource->get_access() == resource_access::dynamic_variable, "Attempted to resize buffer_component_vulkan, but it not resource_access::dynamic_variable. Please submit a bug report.");
		// Let's create a new buffer of the correct size.
		vk2::Buffer& old_buf = this->vk_get_buffer();
		vk2::Buffer new_buf
		{{
			.device = &old_buf.get_device(),
			.size_bytes = sz,
			.usage = old_buf.get_usage(),
			.residency = old_buf.get_residency()
		}};
		// Copy the data over.
		{
			auto old_data = old_buf.map_as<const std::byte>();
			auto new_data = new_buf.map_as<std::byte>();
			std::size_t copy_length = std::min(old_data.size_bytes(), new_data.size_bytes());
			std::copy(old_data.begin(), old_data.begin() + copy_length, new_data.begin());
			this->resource->set_mapped_data(new_data);
		}
		new_buf.debug_set_name(old_buf.debug_get_name());
		std::swap(old_buf, new_buf);
	}

	const vk2::Buffer& buffer_component_vulkan::vk_get_buffer() const
	{
		return this->buffer;
	}

	vk2::Buffer& buffer_component_vulkan::vk_get_buffer()
	{
		return this->buffer;
	}

	bool buffer_component_vulkan::vk_is_descriptor_relevant() const
	{
		return !this->resource->get_flags().contains(resource_flag::index_buffer) && !this->resource->get_flags().contains(resource_flag::draw_indirect_buffer);
	}

	vk2::Buffer buffer_component_vulkan::make_buffer() const
	{
		vk2::BufferUsageField usage_field{vk2::BufferUsage::StorageBuffer};
		vk2::MemoryResidency residency;

		if(this->resource->get_flags().contains(resource_flag::index_buffer))
		{
			usage_field |= vk2::BufferUsage::index_buffer;
		}
		else if(this->resource->get_flags().contains(resource_flag::draw_indirect_buffer))
		{
			usage_field |= vk2::BufferUsage::draw_indirect_buffer;
		}

		switch(this->resource->get_access())
		{
			default:
				tz::error("Unrecognised resource_access. Please submit a bug report.");
			[[fallthrough]];
			case resource_access::static_fixed:
				usage_field |= vk2::BufferUsage::TransferDestination;
				residency = vk2::MemoryResidency::GPU;
			break;
			case resource_access::dynamic_fixed:
			[[fallthrough]];
			case resource_access::dynamic_variable:
				residency = vk2::MemoryResidency::CPUPersistent;
			break;
		}
		return
		{{
			.device = &tz::gl::get_device().vk_get_logical_device(),
			.size_bytes = this->resource->data().size_bytes(),
			.usage = usage_field,
			.residency = residency
		}};
	}

	image_component_vulkan::image_component_vulkan(iresource& resource):
	resource(&resource),
	image(this->make_image()){}

	const iresource* image_component_vulkan::get_resource() const
	{
		return this->resource;
	}

	iresource* image_component_vulkan::get_resource()
	{
		return this->resource;
	}

	tz::vec2ui image_component_vulkan::get_dimensions() const
	{
		return this->image.get_dimensions();
	}

	image_format image_component_vulkan::get_format() const
	{
		return from_vk2(this->image.get_format());
	}

	void image_component_vulkan::resize(tz::vec2ui new_dimensions)
	{
		tz::assert(this->resource->get_access() == resource_access::dynamic_variable, "Requested to resize an image_component_vulkan, but it does not have resource_access::dynamic_variable. Please submit a bug report.");

		// Firstly, make a copy of the old image data.
		std::vector<std::byte> old_data;
		{
			auto old_span = this->resource->data_as<std::byte>();
			old_data.resize(old_span.size_bytes());
			std::copy(old_span.begin(), old_span.end(), old_data.begin());
		}
		// Secondly, let's tell the image_resource the new dimensions. It's data will be invalidated as soon as we do this.
		auto* ires = static_cast<image_resource*>(this->resource);
		ires->set_dimensions(new_dimensions);
		// Then, recreate the image. The image will have the new dimensions but undefined data contents.
		std::string debug_name = this->image.debug_get_name();
		this->image = make_image();
		this->image.debug_set_name(debug_name);
		// After that, let's re-validate the resource data span. It will still have undefined contents for now.
		auto new_data = this->vk_get_image().map_as<std::byte>();
		this->resource->set_mapped_data(new_data);
		// Finally, copy over the old data.
		std::size_t copy_size = std::min(new_data.size_bytes(), old_data.size());
		std::copy(old_data.begin(), old_data.begin() + copy_size, new_data.begin());
		// If the image has grown, then the new texels will still have undefined values because the copy didnt cover the entire contents.
		// Let's zero it all.
		std::size_t num_new_texels = std::max(new_data.size_bytes(), old_data.size()) - copy_size;
		std::fill_n(new_data.begin() + copy_size, num_new_texels, std::byte{0});
	}

	const vk2::Image& image_component_vulkan::vk_get_image() const
	{
		return this->image;
	}

	vk2::Image& image_component_vulkan::vk_get_image()
	{
		return this->image;
	}

	vk2::Image image_component_vulkan::make_image() const
	{
		tz::assert(this->resource->get_type() == resource_type::image, "image_component was provided a resource which was not an image_resource. Please submit a bug report.");
		const image_resource* img_res = static_cast<const image_resource*>(this->resource);
		vk2::ImageUsageField usage_field = {vk2::ImageUsage::TransferDestination, vk2::ImageUsage::SampledImage};
		if(this->resource->get_flags().contains(resource_flag::renderer_output))
		{
			usage_field |= vk2::ImageUsage::ColourAttachment;
			// Let's make sure the format is applicable. We will not allow the user to use an image format that isn't guaranteed to be safe as a colour attachment, even if the machine building the engine allows it. This prevents horrific runtime issues.
			constexpr auto allowed = vk2::format_traits::get_mandatory_colour_attachment_formats();
			if(std::find(allowed.begin(), allowed.end(), to_vk2(img_res->get_format())) == allowed.end())
			{
				tz::error("Detected resource_flag::renderer_output in combination with an image_format that is not guaranteed to work on all GPUs. This may work on some machines, but not others. I cannot allow code to ship which is guaranteed to crash on some devices, sorry.");
			}
		}
		vk2::MemoryResidency residency;
		vk2::ImageTiling tiling = vk2::ImageTiling::Optimal;
		switch(this->resource->get_access())
		{
			default:
				tz::error("Unknown resource_access. Please submit a bug report.");
			[[fallthrough]];
			case resource_access::static_fixed:
				residency = vk2::MemoryResidency::GPU;
			break;
			case resource_access::dynamic_fixed:
			[[fallthrough]];
			case resource_access::dynamic_variable:
				residency = vk2::MemoryResidency::CPUPersistent;
				tiling = vk2::ImageTiling::Linear;
			break;
		}
		return
		{{
			.device = &tz::gl::get_device().vk_get_logical_device(),
			.format = to_vk2(img_res->get_format()),
			.dimensions = img_res->get_dimensions(),
			.usage = usage_field,
			.residency = residency,
			.image_tiling = tiling
		}};
	}
}

#endif // TZ_VULKAN
