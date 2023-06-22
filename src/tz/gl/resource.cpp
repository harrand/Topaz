#include "tz/gl/resource.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/declare/image_format.hpp"
#include <array>

namespace tz::gl
{
	namespace detail
	{
		std::array<const char*, static_cast<int>(resource_access::Count)> resource_access_strings
		{
			"Static",
			"Dynamic",
			"Static Fixed (Legacy)",
			"Dynamic Fixed (Legacy)",
			"Dynamic Variable (Legacy)"
		};

		std::array<const char*, static_cast<int>(resource_flag::Count)> resource_flag_strings
		{
			"Index Buffer",
			"renderer Output",
			"Image Filter: Nearest",
			"Image Filter: Linear",
			"Image Mip Filter: Nearest",
			"Image Mip Filter: Linear",
			"Image Wrap: Clamp To Edge",
			"Image Wrap: Repeat",
			"Image Wrap: Mirrored Repeat"
		};
	}

	resource_type resource::get_type() const
	{
		return this->type;
	}

	resource_access resource::get_access() const
	{
		return this->access;
	}

	const resource_flags& resource::get_flags() const
	{
		return this->flags;
	}

	std::span<const std::byte> resource::data() const
	{
		if(this->mapped_resource_data.has_value())
		{
			return this->mapped_resource_data.value();
		}
		auto beg_offsetted = this->resource_data.begin() + this->initial_alignment_offset;
		return {beg_offsetted, this->resource_data.end()};
	}

	std::span<std::byte> resource::data()
	{
		if(this->mapped_resource_data.has_value())
		{
			return this->mapped_resource_data.value();
		}
		auto beg_offsetted = this->resource_data.begin() + this->initial_alignment_offset;
		return {beg_offsetted, this->resource_data.end()};
	}

	void resource::dbgui()
	{
		const char* type;
		switch(this->get_type())
		{
			case resource_type::buffer:
				type = "Buffer";
			break;
			case resource_type::image:
				type = "Image";
			break;
			default:
				type = "Unknown";
			break;
		}
		ImGui::Text("Resource Type: %s", type);
		ImGui::Text("Resource Access: %s", detail::resource_access_strings[static_cast<int>(this->get_access())]);
		if(!this->get_flags().empty() && ImGui::CollapsingHeader("Resource Flags"))
		{
			ImGui::Indent();
			for(tz::gl::resource_flag flag : this->get_flags())
			{
				ImGui::Text("%s", detail::resource_flag_strings[static_cast<int>(flag)]);
			}
			ImGui::Unindent();
		}
		// use imgui_memory_editor to display resource data.
		static MemoryEditor res_mem_edit;
		static bool show_mem = false;
		if(this->access != tz::gl::resource_access::static_fixed && ImGui::Button("Memory Viewer"))
		{
			show_mem = true;
		}
		if(show_mem)
		{
			ImGui::Begin("Resource Memory Viewer", &show_mem);
			auto dspan = this->data();
			res_mem_edit.DrawContents(dspan.data(), dspan.size_bytes());
			ImGui::End();
		}
	}

	void resource::resize_data(std::size_t new_size)
	{
		this->resource_data.resize(new_size);
	}

	resource::resource(resource_access access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, resource_type type, resource_flags flags):
	access(access),
	resource_data(resource_data),
	mapped_resource_data(std::nullopt),
	initial_alignment_offset(initial_alignment_offset),
	type(type),
	flags(flags)
	{
		tz::assert(!flags.contains(resource_flag::image_mip_nearest) && !flags.contains(resource_flag::image_mip_linear), "Detected resource flag related to image mip filtering. Mips are not yet implemented.");
	}

	void resource::set_mapped_data(std::span<std::byte> mapped_resource_data)
	{
		if(this->get_access() == resource_access::dynamic_access)
		{
			this->mapped_resource_data = mapped_resource_data;
		}
		else
		{
			this->resource_data.resize(mapped_resource_data.size_bytes());
			std::copy(mapped_resource_data.begin(), mapped_resource_data.end(), this->resource_data.begin());
		}
	}

	bool buffer_resource::is_null() const
	{
		return this->data().size_bytes() == 1 && this->data_as<std::byte>().front() == std::byte{255};
	}

	std::unique_ptr<iresource> buffer_resource::unique_clone() const
	{
		return std::make_unique<buffer_resource>(*this);
	}

	void buffer_resource::dbgui()
	{
		resource::dbgui();
		auto size_bytes = this->data().size_bytes();
		ImGui::Text("Buffer Size: ");
		ImGui::SameLine();
		if(size_bytes > (1024*1024*1024))
		{
			ImGui::Text("%.2f GiB", size_bytes / (1024.0f * 1024 * 1024));
		}
		else if(size_bytes > (1024*1024))
		{
			ImGui::Text("%.2f MiB", size_bytes / (1024.0f * 1024));
		}
		else if(size_bytes > 1024)
		{
			ImGui::Text("%.2f KiB", size_bytes / 1024.0f);
		}
		else
		{
			ImGui::Text("%zu B", size_bytes);
		}
	}

	buffer_resource::buffer_resource(resource_access access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, resource_flags flags):
	resource(access, resource_data, initial_alignment_offset, resource_type::buffer, flags){}
			
	image_resource image_resource::from_uninitialised(image_info info)
	{
		std::size_t pixel_size = tz::gl::pixel_size_bytes(info.format);
		std::vector<std::byte> resource_data(pixel_size * info.dimensions[0] * info.dimensions[1]);
		// TODO: Sanity check? Is it correct to just not give a shit about alignment here?
		return {info.access, resource_data, 0, info.format, info.dimensions, info.flags};
	}

	bool image_resource::is_null() const
	{
		const image_resource null = image_resource::null();
		auto null_data = null.data();
		auto my_data = this->data();
		return this->get_dimensions() == null.get_dimensions() && this->get_format() == null.get_format() && std::equal(my_data.begin(), my_data.end(), null_data.begin());
	}

	std::unique_ptr<iresource> image_resource::unique_clone() const
	{
		return std::make_unique<image_resource>(*this);
	}

	void image_resource::dbgui()
	{
		resource::dbgui();
		ImGui::Text("Image Dimensions: {%u, %u}", this->dimensions[0], this->dimensions[1]);
		ImGui::Text("Image Format: %s", detail::image_format_strings[static_cast<int>(this->get_format())]);
	}

	image_format image_resource::get_format() const
	{
		return this->format;
	}

	tz::vec2ui image_resource::get_dimensions() const
	{
		return this->dimensions;
	}

	void image_resource::set_dimensions(tz::vec2ui dims)
	{
		this->dimensions = dims;
	}

	image_resource::image_resource(resource_access access, std::vector<std::byte> resource_data, std::size_t initial_alignment_offset, image_format format, tz::vec2ui dimensions, resource_flags flags):
	resource(access, resource_data, initial_alignment_offset, resource_type::image, flags),
	format(format),
	dimensions(dimensions){}
}
