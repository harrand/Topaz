#include "tz/ren/text.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/imported_shaders.hpp"

#include ImportedShaderHeader(text, vertex)
#include ImportedShaderHeader(text, fragment)

namespace tz::ren
{
//--------------------------------------------------------------------------------------------------
// char_storage
//--------------------------------------------------------------------------------------------------

	char_storage::char_storage(tz::gl::renderer_info& rinfo)
	{
		constexpr std::size_t initial_char_capacity = 1024u;
		std::array<char, initial_char_capacity> initial_chars{0};
		this->char_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_many(initial_chars));
	}

//--------------------------------------------------------------------------------------------------

	char_storage::string_handle char_storage::add_string(tz::gl::renderer_handle rh, std::string str)
	{
		std::span<char> span{str.data(), str.size()};
		char_storage::format(span);
		auto maybe_region = this->try_find_char_region(str.size(), rh);
		if(!maybe_region.has_value())
		{
			const std::size_t old_capacity = this->get_char_capacity(rh);
			const std::size_t new_capacity = std::min(old_capacity + str.size(), old_capacity * 2);
			this->set_char_capacity(rh, new_capacity);
			maybe_region = this->try_find_char_region(str.size(), rh);
			tz::assert(maybe_region.has_value());
		}

		tz::gl::get_device().get_renderer(rh).edit(
			tz::gl::RendererEditBuilder{}
			.write
			({
				.resource = this->char_buffer,
				.data = std::as_bytes(span),
				.offset = maybe_region.value(),
			})
			.build()
		);

		string_locator loc{.offset = maybe_region.value(), .count = str.size()};
		std::size_t string_id = this->strings.size();
		if(this->string_free_list.size())
		{
			string_id = static_cast<std::size_t>(static_cast<tz::hanval>(this->string_free_list.front()));
			this->string_free_list.erase(this->string_free_list.begin());
			this->strings[string_id] = loc;
		}
		else
		{
			this->strings.push_back(loc);
		}
		return static_cast<tz::hanval>(string_id);
	}

//--------------------------------------------------------------------------------------------------

	void char_storage::remove_string(string_handle sh)
	{
		this->string_free_list.push_back(sh);
		this->strings[static_cast<std::size_t>(static_cast<tz::hanval>(sh))] = {};
	}

//--------------------------------------------------------------------------------------------------

	std::size_t char_storage::string_count(bool include_free_list) const
	{
		auto ret = this->strings.size();
		if(include_free_list)
		{
			ret -= this->string_free_list.size();
		}
		return ret;
	}

//--------------------------------------------------------------------------------------------------

	std::optional<std::size_t> char_storage::try_find_char_region(std::size_t char_count, tz::gl::renderer_handle rh) const
	{
		std::vector<string_locator> sorted_strings = this->strings;
		std::sort(sorted_strings.begin(), sorted_strings.end(),
		[](const string_locator& a, const string_locator& b)
		{
			return a.count + a.offset < b.count + b.offset;
		});
		std::size_t current_offset = 0;
		for(const string_locator& loc : sorted_strings)
		{
			tz::assert(loc.offset >= current_offset);
			std::size_t gap_size = loc.offset - current_offset;
			if(gap_size >= char_count)
			{
				return current_offset;
			}
			current_offset = loc.offset + loc.count;
		}
		std::size_t last_string_end = sorted_strings.empty() ? 0 : sorted_strings.back().offset + sorted_strings.back().count;
		if(this->get_char_capacity(rh) - last_string_end >= char_count)
		{
			return last_string_end;
		}
		return std::nullopt;
	}

//--------------------------------------------------------------------------------------------------

	std::size_t char_storage::get_char_capacity(tz::gl::renderer_handle rh) const
	{
		return tz::gl::get_device().get_renderer(rh).get_resource(this->char_buffer)->data().size_bytes();
	}

//--------------------------------------------------------------------------------------------------

	void char_storage::set_char_capacity(tz::gl::renderer_handle rh, std::size_t char_count)
	{
		std::size_t old_cap = this->get_char_capacity(rh);
		if(old_cap <= char_count)
		{
			return;
		}
		tz::gl::get_device().get_renderer(rh).edit(
			tz::gl::RendererEditBuilder{}
			.buffer_resize
			({
				.buffer_handle = this->char_buffer,
				.size = char_count
			})
			.build()
		);
		tz::assert(this->get_char_capacity(rh) == char_count);
	}

//--------------------------------------------------------------------------------------------------
// text_renderer
//--------------------------------------------------------------------------------------------------

	text_renderer::text_renderer(std::size_t image_capacity)
	{
		tz::gl::renderer_info rinfo;
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(text, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(text, fragment));
		this->chars = char_storage{rinfo};

		this->rh = tz::gl::get_device().create_renderer(rinfo);
	}
}