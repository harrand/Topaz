#include "gl/2/resource.hpp"

namespace tz::gl2
{
	ResourceType Resource::get_type() const
	{
		return this->type;
	}

	ResourceAccess Resource::get_access() const
	{
		return ResourceAccess::StaticFixed;
	}

	std::span<const std::byte> Resource::data() const
	{
		auto beg_offsetted = this->resource_data.begin() + this->initial_alignment_offset;
		return {beg_offsetted, this->resource_data.end()};
	}

	std::span<std::byte> Resource::data()
	{
		auto beg_offsetted = this->resource_data.begin() + this->initial_alignment_offset;
		return {beg_offsetted, this->resource_data.end()};
	}

	Resource::Resource(std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, ResourceType type):
	resource_data(resource_data),
	initial_alignment_offset(initial_alignment_offset),
	type(type)
	{}

	BufferResource::BufferResource(std::vector<std::byte> resource_data, std::size_t initial_alignment_offset):
	Resource(resource_data, initial_alignment_offset, ResourceType::Buffer){}
			
	ImageResource ImageResource::from_uninitialised(ImageFormat format, tz::Vec2ui dimensions)
	{
		std::size_t pixel_size = tz::gl2::pixel_size_bytes(format);
		std::vector<std::byte> resource_data(pixel_size * dimensions[0] * dimensions[1]);
		// TODO: Sanity check? Is it correct to just not give a shit about alignment here?
		return {resource_data, 0};
	}

	ImageResource::ImageResource(std::vector<std::byte> resource_data, std::size_t initial_alignment_offset):
	Resource(resource_data, initial_alignment_offset, ResourceType::Image){}
}
