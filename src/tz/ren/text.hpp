#ifndef TOPAZ_REN_TEXT_RENDERER_HPP
#define TOPAZ_REN_TEXT_RENDERER_HPP
#include "tz/core/data/handle.hpp"
#include "tz/core/data/trs.hpp"
#include "tz/core/matrix.hpp"
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
			float pad0 = 0.0f;
			tz::vec3 colour = tz::vec3::filled(1.0f);
			float pad1 = 0.0f;
			tz::mat4 model = tz::mat4::identity();
		};

		string_handle add_string(tz::gl::renderer_handle rh, std::uint32_t font_id, tz::trs transform, tz::vec3 colour, std::string str);
		void remove_string(tz::gl::renderer_handle rh, string_handle sh);
		std::size_t string_count(bool include_free_list = false) const;

		void string_set_transform(tz::gl::renderer_handle rh, string_handle sh, tz::trs transform);
		void string_set_colour(tz::gl::renderer_handle rh, string_handle sh, tz::vec3 colour);
		void string_set_text(tz::gl::renderer_handle rh, string_handle sh, std::string);
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

	/**
	 * @ingroup tz_ren
	 * A 2D text renderer. Use of the text renderer must be externally synchronised.
	 *
	 * Text Renderers are comprised of two major components:
	 * <Font>
	 * - Represents a set of glyphs imported from a TTF font. For each glyph, a MSDF and some spacing information is passed to a shader.
	 * - To add a font, you will need to provide a @ref tz::io::ttf via `add_font`.
	 *		- Please note that `add_font` is very slow, as it must stall the render pipeline, rasterise all the MSDF glyphs and then write them to GPU images. For that reason, you should aim to add all your fonts ahead-of-time.
	 *
	 * <Strings>
	 * - Represents a single rendered text. It can have its own colour and transform (in screen-space)
	 * - To add a text, you will need to invoke `add_string`. You can modify the string later via:
	 * 		- `string_set_transform`, to change the position/rotation/scale of the rendered text.
	 *		- `string_set_colour`, to change the colour of the rendered text.
	 *		- `string_set_text`, to change the actual text being rendered. Note: If the new text has more characters than the old, then the call will take much longer.
	 * - You can remove a text via `remove_string`. 
	 **/
	class text_renderer
	{
	public:
		using string_handle = char_storage::string_handle;
		using font_handle = font_storage::font_handle;

		/**
		 * Create a new text renderer.
		 * @param image_capacity Represents the maximum number of images that can be present at once. The number of images in-use is equal to the number of fonts multiplied by `tz::ren::alphabet.size()tz::ren::alphabet.size()`. If you add too many fonts to exceed this value, the behaviour is undefined.
		 */
		text_renderer(std::size_t image_capacity = 1024u, tz::gl::renderer_options options = {}, tz::gl::ioutput* output = nullptr);

		/// Invoke this every update.
		void update();

		tz::gl::renderer_handle get_render_pass() const;

		/**
		 * Add a new font.
		 * @param font Font to add.
		 * @return Handle corresponding to the created font. Pass this to `add_string` to draw text using this font.
		 */
		font_handle add_font(const tz::io::ttf& font);
		/**
		 * Remove an existing font. Its images can be re-used by another font.
		 * @param fh Font corresponding to the handle to be removed.
		 */
		void remove_font(font_handle fh);

		/**
		 * Add a new rendered text.
		 * @param font Which font should the text be rendered with?
		 * @param transform Where should the text be? This is a transform in screen-space.
		 * @param str String representing the characters to render. Note that control characters, such as line breaks will be rendered as-is and do not function.
		 * @param colour Colour of the rendered text.
		 */
		string_handle add_string(font_handle font, tz::trs transform, std::string str, tz::vec3 colour = tz::vec3::filled(1.0f));
		/**
		 * Remove an existing rendered text.
		 * @param sh String corresponding to the handle to be removed.
		 */
		void remove_string(string_handle sh);

		/**
		 * Set the transform (in screen-space) of an existing rendered text.
		 * @param sh String corresponding to the handle to be affected.
		 * @param transform New transform of the rendered text.
		 */
		void string_set_transform(string_handle sh, tz::trs transform);
		/**
		 * Set the colour of an existing rendered text.
		 * @param sh String corresponding to the handle to be affected.
		 * @param colour New colour of the rendered text.
		 */
		void string_set_colour(string_handle sh, tz::vec3 colour);
		/**
		 * Edit an existing rendered text.
		 * @param sh String corresponding to the handle to be affected.
		 * @param text New characters to be drawn.
		 */
		void string_set_text(string_handle sh, std::string text);

		/// Add the text-renderer to the render-graph, causing it to be invoked every frame.
		void append_to_render_graph();
	private:
		char_storage chars;
		font_storage fonts;
		tz::gl::resource_handle misc_buffer = tz::nullhand;
		tz::gl::renderer_handle rh;
	};
}

#endif // TOPAZ_REN_TEXT_RENDERER_HPP