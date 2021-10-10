#include "gl/resource.hpp"

namespace tz::gl
{
	BufferResource::BufferResource(BufferData data):
	data(data)
	{}

	std::span<const std::byte> BufferResource::get_resource_bytes() const
	{
		return {this->data.data.begin(), this->data.data.end()};
	}

	DynamicBufferResource::DynamicBufferResource(BufferData data):
	initial_data(data),
	resource_data(nullptr)
	{}

	std::span<const std::byte> DynamicBufferResource::get_resource_bytes() const
	{
		if(this->resource_data == nullptr)
		{
			return {this->initial_data.data.begin(), this->initial_data.data.end()};
		}
		return {this->resource_data, this->resource_data + this->initial_data.data.size()};
	}

	std::span<std::byte> DynamicBufferResource::get_resource_bytes_dynamic()
	{
		if(this->resource_data == nullptr)
		{
			return {this->initial_data.data.begin(), this->initial_data.data.end()};
		}
		return {this->resource_data, this->resource_data + this->initial_data.data.size()};
	}

	void DynamicBufferResource::set_resource_data(std::byte* resource_data)
	{
		auto res_data = this->get_resource_bytes();
		this->resource_data = resource_data;
		std::memcpy(this->resource_data, res_data.data(), res_data.size_bytes());
	}

	TextureResource::TextureResource(TextureData data, TextureFormat format, TextureProperties properties):
	data(data),
	format(format),
	properties(properties){}

	std::span<const std::byte> TextureResource::get_resource_bytes() const
	{
		return {this->data.image_data.begin(), this->data.image_data.end()};
	}

	const TextureFormat& TextureResource::get_format() const
	{
		return this->format;
	}

	const TextureProperties& TextureResource::get_properties() const
	{
		return this->properties;
	}

	unsigned int TextureResource::get_width() const
	{
		return data.width;
	}

	unsigned int TextureResource::get_height() const
	{
		return data.height;
	}
}