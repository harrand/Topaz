#include "tz/gl/resource.hpp"

namespace tz::gl
{
	ResourceType Resource::get_type() const
	{
		return this->type;
	}

	ResourceAccess Resource::get_access() const
	{
		return this->access;
	}

	const ResourceFlags& Resource::get_flags() const
	{
		return this->flags;
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

	void Resource::resize_data(std::size_t new_size)
	{
		this->resource_data.resize(new_size);
	}

	Resource::Resource(ResourceAccess access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, ResourceType type, ResourceFlags flags):
	access(access),
	resource_data(resource_data),
	mapped_resource_data(std::nullopt),
	initial_alignment_offset(initial_alignment_offset),
	type(type),
	flags(flags)
	{
		tz_assert(!flags.contains(ResourceFlag::ImageMipNearest) && !flags.contains(ResourceFlag::ImageMipLinear), "Detected resource flag related to image mip filtering. Mips are not yet implemented.");
	}

	void Resource::set_mapped_data(std::span<std::byte> mapped_resource_data)
	{
		tz_assert(this->get_access() == ResourceAccess::DynamicFixed || this->get_access() == ResourceAccess::DynamicVariable, "Cannot set mapped data on a static resource.");
		this->mapped_resource_data = mapped_resource_data;
	}

	std::unique_ptr<IResource> BufferResource::unique_clone() const
	{
		return std::make_unique<BufferResource>(*this);
	}

	BufferResource::BufferResource(ResourceAccess access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, ResourceFlags flags):
	Resource(access, resource_data, initial_alignment_offset, ResourceType::Buffer, flags){}
			
	ImageResource ImageResource::from_uninitialised(ImageInfo info)
	{
		std::size_t pixel_size = tz::gl::pixel_size_bytes(info.format);
		std::vector<std::byte> resource_data(pixel_size * info.dimensions[0] * info.dimensions[1]);
		// TODO: Sanity check? Is it correct to just not give a shit about alignment here?
		return {info.access, resource_data, 0, info.format, info.dimensions, info.flags};
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

	void ImageResource::set_dimensions(tz::Vec2ui dims)
	{
		this->dimensions = dims;
	}

	ImageResource::ImageResource(ResourceAccess access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, ImageFormat format, tz::Vec2ui dimensions, ResourceFlags flags):
	Resource(access, resource_data, initial_alignment_offset, ResourceType::Image, flags),
	format(format),
	dimensions(dimensions){}
}
