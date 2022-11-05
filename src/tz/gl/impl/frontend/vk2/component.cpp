#if TZ_VULKAN
#include "tz/gl/impl/backend/vk2/image_format.hpp"
#include "tz/gl/impl/frontend/vk2/component.hpp"
#include "tz/gl/impl/frontend/vk2/convert.hpp"
#include "tz/gl/resource.hpp"
#include "tz/core/assert.hpp"

namespace tz::gl
{
	using namespace tz::gl;
	BufferComponentVulkan::BufferComponentVulkan(IResource& resource, const vk2::LogicalDevice& ldev):
	resource(&resource),
	buffer(this->make_buffer(ldev))
	{}

	const IResource* BufferComponentVulkan::get_resource() const
	{
		return this->resource;
	}

	IResource* BufferComponentVulkan::get_resource()
	{
		return this->resource;
	}

	std::size_t BufferComponentVulkan::size() const
	{
		tz_assert(this->resource->data().size_bytes() == this->buffer.size(), "BufferComponent Size does not match its IResource data size. Please submit a bug report.");
		return this->buffer.size();
	}

	void BufferComponentVulkan::resize(std::size_t sz)
	{
		(void)sz;
		tz_error("BufferComponent::resize is deprecated. Use a renderer edit instead. If a renderer edit is using this function, it needs to do the resize itself.");
	}

	const vk2::Buffer& BufferComponentVulkan::vk_get_buffer() const
	{
		return this->buffer;
	}

	vk2::Buffer& BufferComponentVulkan::vk_get_buffer()
	{
		return this->buffer;
	}

	bool BufferComponentVulkan::vk_is_descriptor_relevant() const
	{
		return !this->resource->get_flags().contains(ResourceFlag::IndexBuffer) && !this->resource->get_flags().contains(ResourceFlag::DrawIndirectBuffer);
	}

	vk2::Buffer BufferComponentVulkan::make_buffer(const vk2::LogicalDevice& ldev) const
	{
		vk2::BufferUsageField usage_field;
		vk2::MemoryResidency residency;

		vk2::BufferUsage buf_usage;
		if(this->resource->get_flags().contains(ResourceFlag::IndexBuffer))
		{
			buf_usage = vk2::BufferUsage::IndexBuffer;
		}
		else if(this->resource->get_flags().contains(ResourceFlag::DrawIndirectBuffer))
		{
			buf_usage = vk2::BufferUsage::DrawIndirectBuffer;
		}
		else
		{
			buf_usage = vk2::BufferUsage::StorageBuffer;
		}

		if(this->resource->get_access() == ResourceAccess::Static)
		{
			usage_field = {vk2::BufferUsage::TransferDestination, buf_usage};
			residency = vk2::MemoryResidency::GPU;
		}
		else
		{
			usage_field = {buf_usage};
			residency = vk2::MemoryResidency::CPUPersistent;
		}
		return
		{{
			.device = &ldev,
			.size_bytes = this->resource->data().size_bytes(),
			.usage = usage_field,
			.residency = residency
		}};
	}

	ImageComponentVulkan::ImageComponentVulkan(IResource& resource, const vk2::LogicalDevice& ldev):
	resource(&resource),
	image(this->make_image(ldev)){}

	const IResource* ImageComponentVulkan::get_resource() const
	{
		return this->resource;
	}

	IResource* ImageComponentVulkan::get_resource()
	{
		return this->resource;
	}

	tz::Vec2ui ImageComponentVulkan::get_dimensions() const
	{
		return this->image.get_dimensions();
	}

	ImageFormat ImageComponentVulkan::get_format() const
	{
		return from_vk2(this->image.get_format());
	}

	void ImageComponentVulkan::resize(tz::Vec2ui new_dimensions)
	{
		(void)new_dimensions;
		tz_error("ImageComponent::resize is deprecated. Use a renderer edit instead. If a renderer edit is using this function, it needs to do the resize itself.");
		// Firstly, make a copy of the old image data.
		std::vector<std::byte> old_data;
		{
			auto old_span = this->resource->data_as<std::byte>();
			old_data.resize(old_span.size_bytes());
			std::copy(old_span.begin(), old_span.end(), old_data.begin());
		}
		// Secondly, let's tell the ImageResource the new dimensions. It's data will be invalidated as soon as we do this.
		auto* ires = static_cast<ImageResource*>(this->resource);
		ires->set_dimensions(new_dimensions);
		ires->resize_data(new_dimensions[0] * new_dimensions[1] * tz::gl::pixel_size_bytes(tz::gl::from_vk2(this->image.get_format())));
		// Then, recreate the image. The image will have the new dimensions but undefined data contents.
		std::string debug_name = this->image.debug_get_name();
		this->image = make_image(this->image.get_device());
		this->image.debug_set_name(debug_name);
		/*
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
		*/
	}

	const vk2::Image& ImageComponentVulkan::vk_get_image() const
	{
		return this->image;
	}

	vk2::Image& ImageComponentVulkan::vk_get_image()
	{
		return this->image;
	}

	vk2::Image ImageComponentVulkan::make_image(const vk2::LogicalDevice& ldev) const
	{
		tz_assert(this->resource->get_type() == ResourceType::Image, "ImageComponent was provided a resource which was not an ImageResource. Please submit a bug report.");
		const ImageResource* img_res = static_cast<const ImageResource*>(this->resource);
		vk2::ImageUsageField usage_field = {vk2::ImageUsage::TransferDestination, vk2::ImageUsage::SampledImage};
		if(this->resource->get_flags().contains(ResourceFlag::RendererOutput))
		{
			usage_field |= vk2::ImageUsage::ColourAttachment;
			// Let's make sure the format is applicable. We will not allow the user to use an image format that isn't guaranteed to be safe as a colour attachment, even if the machine building the engine allows it. This prevents horrific runtime issues.
			constexpr auto allowed = vk2::format_traits::get_mandatory_colour_attachment_formats();
			if(std::find(allowed.begin(), allowed.end(), to_vk2(img_res->get_format())) == allowed.end())
			{
				tz_error("Detected ResourceFlag::RendererOutput in combination with an ImageFormat that is not guaranteed to work on all GPUs. This may work on some machines, but not others. I cannot allow code to ship which is guaranteed to crash on some devices, sorry.");
			}
		}
		vk2::MemoryResidency residency;
		vk2::ImageTiling tiling = vk2::ImageTiling::Optimal;
		if(this->resource->get_access() == ResourceAccess::Static)
		{
			residency = vk2::MemoryResidency::GPU;
		}
		else
		{
			residency = vk2::MemoryResidency::CPUPersistent;
			tiling = vk2::ImageTiling::Linear;
		}
		return
		{{
			.device = &ldev,
			.format = to_vk2(img_res->get_format()),
			.dimensions = img_res->get_dimensions(),
			.usage = usage_field,
			.residency = residency,
			.image_tiling = tiling
		}};
	}
}

#endif // TZ_VULKAN
