#include "gl/impl/backend/vk2/image_format.hpp"
#if TZ_VULKAN
#include "gl/impl/frontend/vk2/component.hpp"
#include "gl/impl/frontend/vk2/convert.hpp"
#include "gl/resource.hpp"
#include "core/assert.hpp"

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
		return !this->resource->get_flags().contains(ResourceFlag::IndexBuffer);
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
		else
		{
			buf_usage = vk2::BufferUsage::StorageBuffer;
		}

		switch(this->resource->get_access())
		{
			default:
				tz_error("Unrecognised ResourceAccess. Please submit a bug report.");
			[[fallthrough]];
			case ResourceAccess::StaticFixed:
				usage_field = {vk2::BufferUsage::TransferDestination, buf_usage};
				residency = vk2::MemoryResidency::GPU;
			break;
			case ResourceAccess::DynamicFixed:
			[[fallthrough]];
			case ResourceAccess::DynamicVariable:
				usage_field = {buf_usage};
				residency = vk2::MemoryResidency::CPUPersistent;
			break;
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

	const vk2::Image& ImageComponentVulkan::vk_get_image() const
	{
		return this->image;
	}

	vk2::Image& ImageComponentVulkan::vk_get_image()
	{
		return this->image;
	}

	void ImageComponentVulkan::resize(tz::Vec2ui new_dimensions)
	{
		auto* ires = static_cast<ImageResource*>(this->resource);
		ires->set_dimensions(new_dimensions);
		ires->resize_data(tz::gl::pixel_size_bytes(ires->get_format()) * new_dimensions[0] * new_dimensions[1]);
		this->image = make_image(this->image.get_device());
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
		switch(this->resource->get_access())
		{
			default:
				tz_error("Unknown ResourceAccess. Please submit a bug report.");
			[[fallthrough]];
			case ResourceAccess::StaticFixed:
				residency = vk2::MemoryResidency::GPU;
			break;
			case ResourceAccess::DynamicFixed:
			[[fallthrough]];
			case ResourceAccess::DynamicVariable:
				residency = vk2::MemoryResidency::CPUPersistent;
				tiling = vk2::ImageTiling::Linear;
			break;
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
