#include "tz/ren/text.hpp"
#include "tz/core/matrix_transform.hpp"
#include "tz/gl/api/schedule.hpp"
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/imported_shaders.hpp"

#include ImportedShaderHeader(text, vertex)
#include ImportedShaderHeader(text, fragment)
#include ImportedShaderHeader(text, compute)

namespace tz::ren
{
//--------------------------------------------------------------------------------------------------
// char_storage
//--------------------------------------------------------------------------------------------------
	constexpr std::size_t initial_char_capacity = 4096u;

	char_storage::char_storage(tz::gl::renderer_info& rinfo)
	{
		std::array<char, initial_char_capacity> initial_chars{0};
		constexpr std::size_t initial_string_capacity = 64u;
		std::array<string_locator, initial_string_capacity> initial_strings{};
		// char buffer is just a massive array of chars. however, these chars are not ascii.
		// instead, they are represented as alphabet indices (e.g if the alphabet is `abcd`, then `c` (ascii 99) is the 2nd index of that alphabet, thus is 2)
		// this means the shader can use this as an index into the font's glyph array.
		this->char_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_many(initial_chars));
		// string buffer is a array of views into the char buffer. essentially, the char buffer "owns", its chars, and as you add strings to the text renderer, that data is populated and then represents that rendered string.
		// each "string im rendering" has its own element within the string buffer. its our "list of rendered texts"
		this->string_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_many(initial_strings,
		{
			.access = tz::gl::resource_access::dynamic_access
		}));
	}

//--------------------------------------------------------------------------------------------------

	void char_storage::compute_config(tz::gl::renderer_info& cinfo, tz::gl::renderer_handle rh)
	{
		cinfo.ref_resource(rh, this->char_buffer);
		cinfo.ref_resource(rh, this->string_buffer);
	}

//--------------------------------------------------------------------------------------------------

	char_storage::string_handle char_storage::add_string(tz::gl::renderer_handle rh, tz::gl::renderer_handle ch, std::uint32_t font_id, tz::trs transform, tz::vec3 colour, std::string str)
	{
		// first, add all the chars into the buffer.
		std::span<char> span{str.data(), str.size()};
		// remember, we convert from ascii chars to alphabet indices
		char_storage::format(span);
		// we need `str.size()` chars, find an empty region in the char buffer we can populate.
		auto maybe_region = this->try_find_char_region(str.size(), rh);
		if(!maybe_region.has_value())
		{
			// char buffer doesnt have enough space. increase capacity.
			const std::size_t old_capacity = this->get_char_capacity(rh);
			// double or add by string size, whichever is larger.
			const std::size_t new_capacity = std::max(old_capacity + str.size(), old_capacity * 2);
			this->set_char_capacity(rh, new_capacity);
			maybe_region = this->try_find_char_region(str.size(), rh);
			tz::assert(maybe_region.has_value());
		}

		// do char writes.
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

		// now we write the string locator.
		// this is a view into the chars we just wrote.
		// the text will "own" this portion of the char buffer.
		tz::mat4 model = transform.matrix();
		string_locator loc{.offset = static_cast<std::uint32_t>(maybe_region.value()), .count = static_cast<std::uint32_t>(str.size()), .font_id = font_id, .colour = colour, .model = model};
		std::size_t string_id = this->string_cursor;
		if(this->string_free_list.size())
		{
			string_id = static_cast<std::size_t>(static_cast<tz::hanval>(this->string_free_list.front()));
			this->string_free_list.erase(this->string_free_list.begin());
			this->write_string_locator(rh, ch, string_id, loc);
		}
		else
		{
			// run out of string space.
			std::size_t capacity = this->get_string_capacity(rh);
			if(string_id >= this->get_string_capacity(rh))
			{
				// expand. double capacity.
				this->set_string_capacity(rh, capacity * 2);
			}
			this->write_string_locator(rh, ch, string_id, loc);
			this->string_cursor++;
		}
		return static_cast<tz::hanval>(string_id);
	}

//--------------------------------------------------------------------------------------------------

	void char_storage::remove_string(tz::gl::renderer_handle rh, tz::gl::renderer_handle ch, string_handle sh)
	{
		if(std::find(this->string_free_list.begin(), this->string_free_list.end(), sh) != this->string_free_list.end())
		{
			// already in free list. early out.
			return;
		}
		this->string_free_list.push_back(sh);
		// we *must* do the write now.
		// try_find_char_region checks through our locators. if we dont empty that locator out now, we can't recycle that space.
		// its a shame coz remove_string is therefore very slow.
		// possible todo: string buffer should become dynamic_access. remove_string will become very fast but slow down rendering.
		this->write_string_locator(rh, ch, static_cast<std::size_t>(static_cast<tz::hanval>(sh)), {});
	}

//--------------------------------------------------------------------------------------------------

	void char_storage::clear_strings(tz::gl::renderer_handle rh, tz::gl::renderer_handle ch)
	{
		auto sz = this->string_count(true);
		for(std::size_t i = 0; i < sz; i++)
		{
			this->write_string_locator(rh, ch, i, {});
		}
		this->string_free_list.clear();
		this->string_cursor = 0;
	}

//--------------------------------------------------------------------------------------------------

	std::size_t char_storage::string_count(bool include_free_list) const
	{
		auto ret = this->string_cursor;
		if(include_free_list)
		{
			ret -= this->string_free_list.size();
		}
		return ret;
	}

//--------------------------------------------------------------------------------------------------

	void char_storage::string_set_transform(tz::gl::renderer_handle rh, string_handle sh, tz::trs transform)
	{
		auto strings = tz::gl::get_device().get_renderer(rh).get_resource(this->string_buffer)->data_as<string_locator>();
		strings[static_cast<std::size_t>(static_cast<tz::hanval>(sh))].model = transform.matrix();
	}

//--------------------------------------------------------------------------------------------------

	void char_storage::string_set_colour(tz::gl::renderer_handle rh, string_handle sh, tz::vec3 colour)
	{
		auto strings = tz::gl::get_device().get_renderer(rh).get_resource(this->string_buffer)->data_as<string_locator>();
		strings[static_cast<std::size_t>(static_cast<tz::hanval>(sh))].colour = colour;
	}

//--------------------------------------------------------------------------------------------------

	void char_storage::string_set_text(tz::gl::renderer_handle rh, string_handle sh, std::string text)
	{
		auto strings = tz::gl::get_device().get_renderer(rh).get_resource(this->string_buffer)->data_as<string_locator>();
		tz::assert(false, "NYI");
		(void)strings;
		(void)sh;
		(void)text;
	}

//--------------------------------------------------------------------------------------------------

	std::optional<std::size_t> char_storage::try_find_char_region(std::size_t char_count, tz::gl::renderer_handle rh) const
	{
		std::vector<string_locator> sorted_strings(this->get_string_capacity(rh));
		auto resource_data = tz::gl::get_device().get_renderer(rh).get_resource(this->string_buffer)->data_as<const string_locator>();
		std::copy(resource_data.begin(), resource_data.end(), sorted_strings.begin());
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

	void char_storage::update_tri_count(tz::gl::renderer_handle rh, tz::gl::renderer_handle ch) const
	{
		std::size_t char_count = this->get_char_occupancy(rh);
		tz::gl::get_device().get_renderer(rh).edit(tz::gl::RendererEditBuilder{}
		.render_state
		({
			.tri_count = char_count * 2 // each char is a quad, which is 2 triangles
		})
		.build());

		tz::gl::get_device().get_renderer(ch).edit(tz::gl::RendererEditBuilder{}
		.compute
		({
			.kernel = {static_cast<std::uint32_t>(char_count), 1u, 1u}
		})
		.build());
	}

//--------------------------------------------------------------------------------------------------

	void char_storage::write_string_locator(tz::gl::renderer_handle rh, tz::gl::renderer_handle ch, std::size_t string_id, const string_locator& loc)
	{
		// the string buffer is static_access, meaning a renderer edit is necessary to write into it.
		// this is just a helper method to do that.
		auto strings = tz::gl::get_device().get_renderer(rh).get_resource(this->string_buffer)->data_as<string_locator>();
		strings[string_id] = loc;
		this->update_tri_count(rh, ch);
	}

//--------------------------------------------------------------------------------------------------

	std::size_t char_storage::get_char_occupancy(tz::gl::renderer_handle rh) const
	{
		std::size_t total_char_count = 0;
		std::vector<string_locator> locators(this->get_string_capacity(rh));
		auto resource_data = tz::gl::get_device().get_renderer(rh).get_resource(this->string_buffer)->data_as<const string_locator>();
		std::copy(resource_data.begin(), resource_data.end(), locators.begin());
		for(const auto& loc : locators)
		{
			total_char_count += loc.count;
		}
		return total_char_count;
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

	std::size_t char_storage::get_string_capacity(tz::gl::renderer_handle rh) const
	{
		return tz::gl::get_device().get_renderer(rh).get_resource(this->string_buffer)->data().size_bytes() / sizeof(string_locator);
	}

//--------------------------------------------------------------------------------------------------

	void char_storage::set_string_capacity(tz::gl::renderer_handle rh, std::size_t string_count)
	{
		std::size_t old_cap = this->get_string_capacity(rh);
		if(old_cap >= string_count)
		{
			return;
		}
		tz::gl::get_device().get_renderer(rh).edit(
			tz::gl::RendererEditBuilder{}
			.buffer_resize
			({
				.buffer_handle = this->string_buffer,
				.size = string_count * sizeof(string_locator)
			})
			.build()
		);
		tz::assert(this->get_string_capacity(rh) == string_count);
	}

//--------------------------------------------------------------------------------------------------
// font_storage
//--------------------------------------------------------------------------------------------------

	// impl helper class.
	struct font_data
	{
		struct glyph_data
		{
			std::uint32_t image_id;
			float advance;
			tz::vec2 bearing;
		};
		std::array<glyph_data, alphabet.size()> glyphs = {};
	};

//--------------------------------------------------------------------------------------------------

	font_storage::font_storage(tz::gl::renderer_info& rinfo, std::size_t image_capacity)
	{
		constexpr std::size_t initial_font_capacity = 8;
		std::array<font_data, initial_font_capacity> initial_font_data;
		this->font_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_many(initial_font_data));

		for(std::size_t i = 0; i < image_capacity; i++)
		{
			this->images.push_back(rinfo.add_resource(tz::gl::image_resource::from_uninitialised
			({
				.format = tz::gl::image_format::RGBA32,
				.dimensions = {1u, 1u},
				.flags = {tz::gl::resource_flag::image_filter_linear}
			})));
		}	
	}

//--------------------------------------------------------------------------------------------------

	void font_storage::compute_config(tz::gl::renderer_info& cinfo, tz::gl::renderer_handle rh)
	{
		cinfo.ref_resource(rh, this->font_buffer);
		// dont need access to images.
	}

//--------------------------------------------------------------------------------------------------

	font_storage::font_handle font_storage::add_font(const tz::io::ttf& font, tz::gl::renderer_handle rh)
	{
		tz::gl::RendererEditBuilder builder;
		font_data d;
		std::vector<tz::io::image> images;
		images.resize(alphabet.size());

		// which font id are we?
		std::size_t font_id = this->font_cursor;
		if(this->font_free_list.size())
		{
			font_id = static_cast<std::size_t>(static_cast<tz::hanval>(this->font_free_list.front()));
			this->font_free_list.erase(this->font_free_list.begin());
		}
		else
		{
			this->font_cursor++;
		}
		// for each glyph in the font, write into a chosen image id.
		for(std::size_t i = 0; i < alphabet.size(); i++)
		{
			std::size_t image_id = this->image_cursor;
			if(this->image_free_list.size())
			{
				// re-use an old image.
				image_id = static_cast<std::size_t>(static_cast<tz::hanval>(this->image_free_list.front()));
				this->image_free_list.erase(this->image_free_list.begin());
			}
			else
			{
				// gotta increment our image cursor.
				// remember, the image capacity is fixed at construction. if we exceed this, we cant resize - we're fucked.
				tz::assert(this->image_cursor < this->images.size(), "ran out of images");
				this->image_cursor++;
			}

			char c = alphabet[i];
			// love me some magic numbers.
			// get the glyph information we need from the ttf.
			auto iter = font.get_glyphs().find(c);
			if(iter == font.get_glyphs().end())
			{
				// if the ttf doesnt support this glyph, pretend its a question mark.
				iter = font.get_glyphs().find('?');
				// if the ttf doesn't support ? either, then we're really screwed. boom.
				tz::assert(iter != font.get_glyphs().end());
			}
			const auto& glyph_data = iter->second;
			d.glyphs[i].image_id = image_id;
			d.glyphs[i].advance = glyph_data.spacing.advance;
			d.glyphs[i].bearing = glyph_data.spacing.position;

			images[i] = font.rasterise_msdf(c,
			{
				.dimensions = {32u, 32u},
				.angle_threshold = 3.0f,
				.range = 0.2f,
				.scale = 48.0f,
				.translate = static_cast<tz::vec2>(glyph_data.spacing.position) * -1.0f
			});
			// resize + write the new image data.
			auto imgdata = std::span<const std::byte>(images[i].data);
			// note: we pre-resized `images`, so dont worry about lifetime issues of the span.
			builder
			.image_resize
			({
				.image_handle = this->images[image_id],
				.dimensions = {images[i].width, images[i].height}
			})
			.write
			({
				.resource = this->images[image_id],
				.data = imgdata,
				.offset = 0u
			});

		}	

		// now write the new font data.
		builder.write
		({
			.resource = this->font_buffer,
			.data = std::as_bytes(std::span<const font_data>(&d, 1)),
			.offset = font_id * sizeof(font_data)
		});
		tz::gl::get_device().get_renderer(rh).edit(builder.build());
		return static_cast<tz::hanval>(font_id);
	}

//--------------------------------------------------------------------------------------------------

	void font_storage::remove_font(font_handle fh, tz::gl::renderer_handle rh)
	{
		this->font_free_list.push_back(fh);
		// also "free" all our images.
		const auto& fonts = tz::gl::get_device().get_renderer(rh).get_resource(this->font_buffer)->data_as<const font_data>();
		const font_data& font = fonts[static_cast<std::size_t>(static_cast<tz::hanval>(fh))];
		for(const auto& glyph : font.glyphs)
		{
			this->image_free_list.push_back(static_cast<tz::hanval>(glyph.image_id));
		}
		// todo: remove all texts using this font?
	}

//--------------------------------------------------------------------------------------------------
// text_renderer
//--------------------------------------------------------------------------------------------------

	struct misc_data
	{
		tz::vec2 mondims;
	};

	struct glyph_render_data
	{
		font_data::glyph_data chardata;
		std::uint32_t string_id = static_cast<std::uint32_t>(-1);
	};

	text_renderer::text_renderer(std::size_t image_capacity, tz::gl::renderer_options options, tz::gl::ioutput* output)
	{
		tz::gl::renderer_info rinfo;
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(text, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(text, fragment));
		rinfo.set_options(options);
		if(output != nullptr)
		{
			rinfo.set_output(*output);
		}
		this->misc_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_one(misc_data
		{
			.mondims = tz::window().get_dimensions()
		},
		{
			.access = tz::gl::resource_access::dynamic_access
		}));
		std::array<glyph_render_data, initial_char_capacity> render_buffer_data;
		this->render_buffer = rinfo.add_resource(tz::gl::buffer_resource::from_many(render_buffer_data));

		this->chars = char_storage{rinfo};
		this->fonts = font_storage{rinfo, image_capacity};

		rinfo.debug_name("Text Renderer");

		this->rh = tz::gl::get_device().create_renderer(rinfo);

		// compute
		tz::gl::renderer_info cinfo;
		cinfo.shader().set_shader(tz::gl::shader_stage::compute, ImportedShaderSource(text, compute));
		this->chars.compute_config(cinfo, this->rh);
		this->fonts.compute_config(cinfo, this->rh);
		cinfo.ref_resource(this->rh, this->render_buffer);
		cinfo.debug_name("Text Renderer - Compute Prepass");
		cinfo.set_options({tz::gl::renderer_option::render_wait});
		this->ch = tz::gl::get_device().create_renderer(cinfo);
	}

//--------------------------------------------------------------------------------------------------

	void text_renderer::update()
	{
		misc_data& misc = tz::gl::get_device().get_renderer(this->rh).get_resource(this->misc_buffer)->data_as<misc_data>().front();
		misc.mondims = tz::window().get_dimensions();
	}

//--------------------------------------------------------------------------------------------------

	tz::gl::renderer_handle text_renderer::get_render_pass() const
	{
		return this->rh;
	}

//--------------------------------------------------------------------------------------------------

	text_renderer::font_handle text_renderer::add_font(const tz::io::ttf& font)
	{
		return this->fonts.add_font(font, this->rh);
	}

//--------------------------------------------------------------------------------------------------

	void text_renderer::remove_font(font_handle fh)
	{
		this->fonts.remove_font(fh, this->rh);
	}

//--------------------------------------------------------------------------------------------------

	text_renderer::string_handle text_renderer::add_string(font_handle font, tz::trs transform, std::string str, tz::vec3 colour)
	{
		auto ret = this->chars.add_string(this->rh, this->ch, static_cast<std::size_t>(static_cast<tz::hanval>(font)), transform, colour, str);
		tz::gl::get_device().get_renderer(this->ch).render();
		return ret;
	}

//--------------------------------------------------------------------------------------------------

	void text_renderer::remove_string(string_handle sh)
	{
		this->chars.remove_string(this->rh, this->ch, sh);
		tz::gl::get_device().get_renderer(this->ch).render();
	}

//--------------------------------------------------------------------------------------------------

	void text_renderer::clear_strings()
	{
		this->chars.clear_strings(this->rh, this->ch);
		tz::gl::get_device().get_renderer(this->ch).render();
	}

//--------------------------------------------------------------------------------------------------

	void text_renderer::string_set_transform(string_handle sh, tz::trs transform)
	{
		this->chars.string_set_transform(this->rh, sh, transform);
	}

//--------------------------------------------------------------------------------------------------

	void text_renderer::string_set_colour(string_handle sh, tz::vec3 colour)
	{
		this->chars.string_set_colour(this->rh, sh, colour);
	}

//--------------------------------------------------------------------------------------------------

	void text_renderer::string_set_text(string_handle sh, std::string text)
	{
		this->chars.string_set_text(this->rh, sh, text);
		tz::gl::get_device().get_renderer(this->ch).render();
	}

//--------------------------------------------------------------------------------------------------

	void text_renderer::append_to_render_graph()
	{
		//tz::gl::get_device().render_graph().timeline.push_back(static_cast<tz::gl::eid_t>(static_cast<std::size_t>(static_cast<tz::hanval>(this->ch))));
		tz::gl::get_device().render_graph().timeline.push_back(static_cast<tz::gl::eid_t>(static_cast<std::size_t>(static_cast<tz::hanval>(this->rh))));
		//tz::gl::get_device().render_graph().add_dependencies(this->rh, this->ch);
	}
}