#include "gl/resource.hpp"

namespace tz::gl2
{
	ResourceType Resource::get_type() const
	{
		return this->type;
	}

	ResourceAccess Resource::get_access() const
	{
		return this->access;
	}

	std::span<const std::byte> Resource::data() const
	{
		if(this->mapped_resource_data.has_value())
		{
			return this->mapped_resource_data.value();
		}
		auto beg_offsetted = this->resource_data.begin() + this->initial_alignment_offset;
		return {beg_offsetted, this->resource_data.end()};
	}

	std::span<std::byte> Resource::data()
	{
		if(this->mapped_resource_data.has_value())
		{
			return this->mapped_resource_data.value();
		}
		auto beg_offsetted = this->resource_data.begin() + this->initial_alignment_offset;
		return {beg_offsetted, this->resource_data.end()};
	}

	Resource::Resource(ResourceAccess access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, ResourceType type):
	access(access),
	resource_data(resource_data),
	mapped_resource_data(std::nullopt),
	initial_alignment_offset(initial_alignment_offset),
	type(type)
	{}

	void Resource::set_mapped_data(std::span<std::byte> mapped_resource_data)
	{
		tz_assert(this->get_access() == ResourceAccess::DynamicFixed || this->get_access() == ResourceAccess::DynamicVariable, "Cannot set mapped data on a static resource.");
		this->mapped_resource_data = mapped_resource_data;
	}

	std::unique_ptr<IResource> BufferResource::unique_clone() const
	{
		return std::make_unique<BufferResource>(*this);
	}

	BufferResource::BufferResource(ResourceAccess access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset):
	Resource(access, resource_data, initial_alignment_offset, ResourceType::Buffer){}
			
	ImageResource ImageResource::from_uninitialised(ImageFormat format, tz::Vec2ui dimensions, ResourceAccess access)
	{
		std::size_t pixel_size = tz::gl2::pixel_size_bytes(format);
		std::vector<std::byte> resource_data(pixel_size * dimensions[0] * dimensions[1]);
		// TODO: Sanity check? Is it correct to just not give a shit about alignment here?
		return {access, resource_data, 0, format, dimensions};
	}

	std::unique_ptr<IResource> ImageResource::unique_clone() const
	{
		return std::make_unique<ImageResource>(*this);
	}

	ImageFormat ImageResource::get_format() const
	{
		return this->format;
	}

	tz::Vec2ui ImageResource::get_dimensions() const
	{
		return this->dimensions;
	}

	ImageResource::ImageResource(ResourceAccess access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, ImageFormat format, tz::Vec2ui dimensions):
	Resource(access, resource_data, initial_alignment_offset, ResourceType::Image),
	format(format),
	dimensions(dimensions){}
}
