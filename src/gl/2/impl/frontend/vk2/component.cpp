#if TZ_VULKAN
#include "gl/2/impl/frontend/vk2/component.hpp"
#include "gl/2/impl/frontend/vk2/convert.hpp"
#include "gl/2/resource.hpp"
#include "core/assert.hpp"

namespace tz::gl2
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

	vk2::Buffer& BufferComponentVulkan::vk_get_buffer()
	{
		return this->buffer;
	}

	vk2::Buffer BufferComponentVulkan::make_buffer(const vk2::LogicalDevice& ldev) const
	{
		vk2::BufferUsageField usage_field;
		vk2::MemoryResidency residency;
		switch(this->resource->get_access())
		{
			case ResourceAccess::StaticFixed:
			[[fallthrough]];
			case ResourceAccess::StaticVariable:
				usage_field = {vk2::BufferUsage::TransferDestination, vk2::BufferUsage::StorageBuffer};
				residency = vk2::MemoryResidency::GPU;
			break;
			case ResourceAccess::DynamicFixed:
			[[fallthrough]];
			case ResourceAccess::DynamicVariable:
				usage_field = {vk2::BufferUsage::StorageBuffer};
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

	vk2::Image& ImageComponentVulkan::vk_get_image()
	{
		return this->image;
	}

	vk2::Image ImageComponentVulkan::make_image(const vk2::LogicalDevice& ldev) const
	{
		tz_assert(this->resource->get_type() == ResourceType::Image, "ImageComponent was provided a resource which was not an ImageResource. Please submit a bug report.");
		const ImageResource* img_res = static_cast<const ImageResource*>(this->resource);
		vk2::ImageUsageField usage_field;
		vk2::MemoryResidency residency;
		switch(this->resource->get_access())
		{
			case ResourceAccess::StaticFixed:
			[[fallthrough]];
			case ResourceAccess::StaticVariable:
				usage_field = {vk2::ImageUsage::TransferDestination, vk2::ImageUsage::SampledImage};
				residency = vk2::MemoryResidency::GPU;
			break;
			case ResourceAccess::DynamicFixed:
			[[fallthrough]];
			case ResourceAccess::DynamicVariable:
				usage_field = {vk2::ImageUsage::StorageImage};
				residency = vk2::MemoryResidency::CPUPersistent;
			break;
		}
		return
		{{
			.device = &ldev,
			.format = to_vk2(img_res->get_format()),
			.dimensions = img_res->get_dimensions(),
			.usage = usage_field,
			.residency = residency
		}};
	}
}

#endif // TZ_VULKAN
