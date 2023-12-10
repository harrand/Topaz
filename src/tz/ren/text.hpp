#ifndef TOPAZ_REN_TEXT_RENDERER_HPP
#define TOPAZ_REN_TEXT_RENDERER_HPP
#include "tz/core/data/handle.hpp"
#include "tz/gl/api/resource.hpp"
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
			std::uint32_t offset = 0;
			std::uint32_t count = 0;
			std::uint32_t font_id = 0;
			float pad0;
			tz::vec2 position = tz::vec2::zero();
		};

		string_handle add_string(tz::gl::renderer_handle rh, std::uint32_t font_id, tz::vec2 position, std::string str);
		void remove_string(tz::gl::renderer_handle rh, string_handle sh);
		std::size_t string_count(bool include_free_list = false) const;
	private:
		std::optional<std::size_t> try_find_char_region(std::size_t char_count, tz::gl::renderer_handle rh) const;
		void update_tri_count(tz::gl::renderer_handle rh) const;
		void write_string_locator(tz::gl::renderer_handle rh, std::size_t string_id, const string_locator& loc);
		std::size_t get_char_occupancy(tz::gl::renderer_handle rh) const;
		std::size_t get_char_capacity(tz::gl::renderer_handle rh) const;
		void set_char_capacity(tz::gl::renderer_handle rh, std::size_t char_count);
		std::size_t get_string_capacity(tz::gl::renderer_handle rh) const;
		void set_string_capacity(tz::gl::renderer_handle rh, std::size_t string_count);
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
		tz::gl::resource_handle string_buffer = tz::nullhand;
		std::size_t string_cursor = 0;
		std::vector<string_handle> string_free_list = {};
	};

	class font_storage
	{
	public:
		font_storage(tz::gl::renderer_info& rinfo, std::size_t image_capacity);
		font_storage() = default;
		using font_handle = tz::handle<tz::io::ttf>;

		font_handle add_font(const tz::io::ttf& font, tz::gl::renderer_handle rh);
		void remove_font(font_handle fh, tz::gl::renderer_handle rh);
	private:
		tz::gl::resource_handle font_buffer = tz::nullhand;
		std::vector<tz::gl::resource_handle> images = {};
		std::size_t image_cursor = 0;
		std::vector<tz::gl::resource_handle> image_free_list = {};
		std::size_t font_cursor = 0;
		std::vector<font_handle> font_free_list = {};
	};

	class text_renderer
	{
	public:
		using string_handle = char_storage::string_handle;
		using font_handle = font_storage::font_handle;

		text_renderer(std::size_t image_capacity = 1024u);

		font_handle add_font(const tz::io::ttf& font);
		void remove_font(font_handle fh);

		// api todo: how to specify which font? remember char_storage has no concept of fonts, so simply adding a font_handle to string_locator is not that simple.
		string_handle add_string(font_handle font, tz::vec2 position, std::string str);
		void remove_string(string_handle sh);

		void append_to_render_graph();
	private:
		char_storage chars;
		font_storage fonts;
		tz::gl::renderer_handle rh;
	};
}

#endif // TOPAZ_REN_TEXT_RENDERER_HPP