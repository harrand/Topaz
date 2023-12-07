#include "tz/ren/text.hpp"
#include "tz/gl/device.hpp"

namespace tz::ren
{
	std::uint32_t font_entry::get_glyph_image(char ch) const
	{
		// given char `ch`, return the image id corresponding to the glyph within this font.
		auto maybe_idx = alphabet_indexof(ch);
		tz::assert(maybe_idx.has_value());
		return this->alphabet_images[maybe_idx.value()];
	}

	text_renderer::text_renderer(std::size_t image_capacity)
	{
		tz::gl::renderer_info rinfo;
		this->images.resize(image_capacity, tz::nullhand);
		for(std::size_t i = 0; i < image_capacity; i++)
		{
			this->images[i] = rinfo.add_resource(tz::gl::image_resource::from_uninitialised(
				{
					.format = tz::gl::image_format::RGBA32,
					.dimensions = {1u, 1u}
			}));
		}
	}

	text_renderer::font_handle text_renderer::add_font(tz::io::ttf font)
	{
		std::size_t font_id = this->fonts.size();
		// font id is either font count so far, or free-list.
		if(this->font_free_list.size())
		{
			font_id = static_cast<std::size_t>(static_cast<tz::hanval>(this->font_free_list.front()));
			this->font_free_list.erase(this->font_free_list.begin());
		}

		auto& entry = this->fonts.emplace_back();
		for(char c : alphabet)
		{
			std::uint32_t texture_id = this->texture_cursor;
			// find a texture id for us to use.
			if(this->image_id_free_list.size())
			{
				texture_id = this->image_id_free_list.front();
				this->image_id_free_list.erase(this->image_id_free_list.begin());
			}
			else
			{
				tz::assert(this->texture_cursor < this->images.size(), "ran out of images");
				this->texture_cursor++;
			}
			// tell the entry which image index it can use
			auto maybe_idx = alphabet_indexof(c);
			tz::assert(maybe_idx.has_value());
			entry.alphabet_images[maybe_idx.value()] = texture_id;
			// write the image.
			tz::io::image img = font.rasterise_msdf(c, {});
			auto data = std::span<const std::byte>(img.data);

			tz::gl::get_device().get_renderer(this->rh).edit(tz::gl::RendererEditBuilder{}
			.image_resize
			({
				.image_handle = static_cast<tz::hanval>(texture_id),
				.dimensions = {img.width, img.height}
			})
			.write
			({
				.resource = static_cast<tz::hanval>(texture_id),
				.data = data,
				.offset = 0u,
			})
			.build());
		}
		
		return static_cast<tz::hanval>(font_id);
	}

	void text_renderer::remove_font(font_handle fh)
	{
		auto font_id = static_cast<std::size_t>(static_cast<tz::hanval>(fh));
		// recycle all its images
		for(std::uint32_t image_id : this->fonts[font_id].alphabet_images)
		{
			this->image_id_free_list.push_back(image_id);
		}
		// set to empty
		this->fonts[font_id] = {};
		// recycle me.
		this->font_free_list.push_back(fh);
	}
}