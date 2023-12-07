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

	struct font_entry
	{
		std::uint32_t get_glyph_image(char ch) const;

		// alphabet_images is a list of image ids. [0] means the image corresponding to the glyph alphabet[0]
		std::array<std::uint32_t, alphabet.length()> alphabet_images = {};
	};

	class text_renderer
	{
	public:
		text_renderer(std::size_t image_capacity = 1024u);
		using font_handle = tz::handle<font_entry>;

		font_handle add_font(tz::io::ttf font);
		void remove_font(font_handle fh);
	private:
		tz::gl::renderer_handle rh = tz::nullhand;
		// stores a set of text structures. each text has an offset + length view into the char buffer.
		//tz::gl::resource_handle text_buffer = tz::nullhand;
		// stores a massive list of chars (encoded not as ascii, but as alphabet indices). all text data lives here.
		//tz::gl::resource_handle char_buffer = tz::nullhand;
		std::vector<tz::gl::resource_handle> images = {};
		std::size_t texture_cursor = 0;
		std::vector<std::uint32_t> image_id_free_list = {};
		std::vector<font_entry> fonts = {};
		std::vector<font_handle> font_free_list = {};
	};
}

#endif // TOPAZ_REN_TEXT_RENDERER_HPP