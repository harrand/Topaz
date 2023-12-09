#ifndef TOPAZ_REN_TEXT_RENDERER_HPP
#define TOPAZ_REN_TEXT_RENDERER_HPP
#include "tz/core/data/handle.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/io/ttf.hpp"

namespace tz::ren
{
	constexpr std::string_view alphabet = tz::io::ttf_alphabet;
	constexpr std::optional<std::size_t> alphabet_indexof(char ch)
	{
		auto id = alphabet.find(ch);
		if(id != std::string_view::npos)
		{
			return id;
		}
		return std::nullopt;
	}

	class char_storage
	{
	public:
		char_storage(tz::gl::renderer_info& rinfo);
		char_storage() = default;
		using string_handle = tz::handle<const char*>;
		struct string_locator
		{
			std::size_t offset = 0;
			std::size_t count = 0;
		};

		string_handle add_string(tz::gl::renderer_handle rh, std::string str);
		void remove_string(string_handle sh);
		std::size_t string_count(bool include_free_list = false) const;
	private:
		std::optional<std::size_t> try_find_char_region(std::size_t char_count, tz::gl::renderer_handle rh) const;
		std::size_t get_char_capacity(tz::gl::renderer_handle rh) const;
		void set_char_capacity(tz::gl::renderer_handle rh, std::size_t char_count);
		// convert ascii chars into alphabet indices (in-place)
		static constexpr void format(std::span<char> ascii_str)
		{
			for(char& c : ascii_str)
			{
				auto maybe_index = alphabet_indexof(c);
				if(!maybe_index.has_value())
				{
					maybe_index = alphabet_indexof('?');
					tz::assert(maybe_index.has_value());
				}
				c = maybe_index.value();
			}
		}

		tz::gl::resource_handle char_buffer = tz::nullhand;
		std::vector<string_locator> strings = {};
		std::vector<string_handle> string_free_list = {};
	};

	class text_renderer
	{
	public:
		text_renderer(std::size_t image_capacity = 1024u);
	private:
		char_storage chars;
		tz::gl::renderer_handle rh;
	};
}

#endif // TOPAZ_REN_TEXT_RENDERER_HPP