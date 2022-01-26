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

	std::unique_ptr<IResource> BufferResource::unique_clone() const
	{
		return std::make_unique<BufferResource>(*this);
	}

	BufferResource::BufferResource(std::vector<std::byte> resource_data, std::size_t initial_alignment_offset):
	Resource(resource_data, initial_alignment_offset, ResourceType::Buffer){}
			
	ImageResource ImageResource::from_uninitialised(ImageFormat format, tz::Vec2ui dimensions)
	{
		std::size_t pixel_size = tz::gl2::pixel_size_bytes(format);
		std::vector<std::byte> resource_data(pixel_size * dimensions[0] * dimensions[1]);
		// TODO: Sanity check? Is it correct to just not give a shit about alignment here?
		return {resource_data, 0, format, dimensions};
	}

	ImageResource ImageResource::from_memory(ImageFormat format, tz::Vec2ui dimensions, std::span<const std::byte> byte_data)
	{
		std::size_t pixel_size = tz::gl2::pixel_size_bytes(format);
		std::vector<std::byte> resource_data(pixel_size * dimensions[0] * dimensions[1]);
		std::copy(byte_data.begin(), byte_data.end(), resource_data.begin());
		// TODO: Sanity check? Is it correct to just not give a shit about alignment here?
		return {resource_data, 0, format, dimensions};
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

	ImageResource::ImageResource(std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, ImageFormat format, tz::Vec2ui dimensions):
	Resource(resource_data, initial_alignment_offset, ResourceType::Image),
	format(format),
	dimensions(dimensions){}
}
