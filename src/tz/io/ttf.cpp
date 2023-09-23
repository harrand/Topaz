#include "tz/io/ttf.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
#include "tz/core/endian.hpp"
#include <fstream>
#include <cstdio>
#include <cstring>

namespace tz::io
{
	template<std::integral T>
	T ttf_read_value(const char*& data)
	{
		T ret = tz::big_endian(*reinterpret_cast<const T*>(data));
		data += sizeof(T);
		return ret;
	}

	ttf ttf::from_memory(std::string_view sv)
	{
		TZ_PROFZONE("ttf - from memory", 0xFFFF2222);
		return {sv};
	}

	ttf ttf::from_file(const char* path)
	{
		TZ_PROFZONE("ttf - from file", 0xFFFF2222);
		std::ifstream file(path, std::ios::binary);
		tz::assert(file.good(), "Could not load ttf from file because the path was wrong, or something else went wrong.");
		std::string buffer(std::istreambuf_iterator<char>(file), {});
		return ttf::from_memory(buffer);
	}

	ttf::ttf(std::string_view ttf_data)
	{
		std::string_view ttf_minus_header = this->parse_header(ttf_data);
		this->parse_header(ttf_data);
		this->parse_table_info(ttf_minus_header, ttf_data);
		this->parse_tables(ttf_data);
		// by the time we've parsed tables, we expect all the tables to be sorted.
		// let's do a sanity check ensuring that the canary of the head table has been set to true.
		tz::assert(this->head.canary, "TTF Head Table canary value was never set to true, this means that a head table was not located. Most likely the TTF is malformed or corrupted.");
		tz::assert(this->maxp.canary, "TTF maxp Table canary value was never set to true, this means that a head table was not located. Most likely the TTF is malformed or corrupted.");
		tz::assert(this->hhea.canary, "TTF hhea Table canary value was never set to true, this means that a head table was not located. Most likely the TTF is malformed or corrupted.");
		tz::assert(this->hmtx.canary, "TTF hmtx Table canary value was never set to true, this means that a head table was not located. Most likely the TTF is malformed or corrupted.");
		tz::assert(this->loca.canary, "TTF loca Table canary value was never set to true, this means that a head table was not located. Most likely the TTF is malformed or corrupted.");
		tz::assert(this->glyf.canary, "TTF glyf Table canary value was never set to true, this means that a head table was not located. Most likely the TTF is malformed or corrupted.");
		tz::assert(this->cmap.canary, "TTF cmap Table canary value was never set to true, this means that a head table was not located. Most likely the TTF is malformed or corrupted.");
	}

	std::string_view ttf::parse_header(std::string_view str)
	{
		const char* ptr = str.data();
		this->header.scalar_type = ttf_read_value<std::uint32_t>(ptr);
		this->header.num_tables = ttf_read_value<std::uint16_t>(ptr);
		this->header.search_range = ttf_read_value<std::uint16_t>(ptr);
		this->header.entry_selector = ttf_read_value<std::uint16_t>(ptr);
		this->header.range_shift = ttf_read_value<std::uint16_t>(ptr);

		auto byte_diff = std::distance(str.data(), ptr);
		tz::assert(std::cmp_greater(str.size(), byte_diff));
		str.remove_prefix(byte_diff);
		return str;
	}

	void ttf::parse_table_info(std::string_view str, std::string_view full_data)
	{
		const char* ptr = str.data();
		tz::assert(this->tables.empty());
		for(std::size_t i = 0; i < this->header.num_tables; i++)
		{
			ttf_table& tbl = this->tables.emplace_back();
			std::memcpy(tbl.tag, ptr, 4);
			ptr += 4;	
			tbl.checksum = ttf_read_value<std::uint32_t>(ptr);
			tbl.offset = ttf_read_value<std::uint32_t>(ptr);
			tbl.length = ttf_read_value<std::uint32_t>(ptr);

			tz::assert(std::string{tbl.tag} != "DEAD", "Detected no change to table tag - malformed TTF.");
			std::string tagstr{tbl.tag};
			if(tagstr != "head")
			{
				std::uint32_t calc = calculate_table_checksum(full_data, tbl.offset, tbl.length);
				tz::assert(calc == tbl.checksum);
			}
		}
	}

	ttf_table ttf::find_table_by_tag(const char* tag) const
	{
		for(const ttf_table& t : this->tables)
		{
			std::string tagstr{t.tag};
			if(tagstr == tag)
			{
				return t;
			}
		}
		tz::error("Could not find table with tag %s", tag);
		return {};
	}

	void ttf::parse_tables(std::string_view data)
	{
		this->parse_head_table(data, this->find_table_by_tag("head"));
		this->parse_maxp_table(data, this->find_table_by_tag("maxp"));
		this->parse_hhea_table(data, this->find_table_by_tag("hhea"));
		this->parse_hmtx_table(data, this->find_table_by_tag("hmtx"));
		this->parse_loca_table(data, this->find_table_by_tag("loca"));
		this->parse_glyf_table(data, this->find_table_by_tag("glyf"));
		this->parse_cmap_table(data, this->find_table_by_tag("cmap"));
	}

	std::uint32_t ttf::calculate_table_checksum(std::string_view data, std::uint32_t offset, std::uint32_t length) const
	{
		std::uint32_t sum = 0u;
		for(std::size_t i = 0; i < length; i += sizeof(std::uint32_t))
		{
			sum += tz::big_endian(*reinterpret_cast<const std::uint32_t*>(data.data() + offset + i));
		}
		return sum;
	}

	void ttf::parse_head_table(std::string_view data, ttf_table table_descriptor)
	{
		tz::assert(data.size() > table_descriptor.offset + table_descriptor.length);
		data.remove_prefix(table_descriptor.offset);
		data.remove_suffix(data.size() - table_descriptor.length);
		tz::assert(data.size() == (table_descriptor.length));

		tz::assert(!this->head.canary, "When parsing head table, noticed canary already switched to true. Double head table discovery? Most likely malformed TTF.");

		const char* ptr = data.data();

		this->head.major_version = ttf_read_value<std::uint16_t>(ptr);

		this->head.minor_version = ttf_read_value<std::uint16_t>(ptr);

		this->head.font_revision_fixed_point = ttf_read_value<std::int32_t>(ptr);
		// fixed-point conversion: divide by (1 >> 16)
		this->head.font_revision_fixed_point /= (1 << 16);
		this->head.checksum_adjustment = ttf_read_value<std::uint32_t>(ptr);
		this->head.magic = ttf_read_value<std::uint32_t>(ptr);
		this->head.flags = ttf_read_value<std::uint16_t>(ptr);
		this->head.units_per_em = ttf_read_value<std::uint16_t>(ptr);
		// ignore created and modified date for now. aids.
		ptr += sizeof(std::uint64_t) * 2;
		this->head.xmin = ttf_read_value<std::int16_t>(ptr);
		this->head.ymin = ttf_read_value<std::int16_t>(ptr);
		this->head.xmax = ttf_read_value<std::int16_t>(ptr);
		this->head.ymax = ttf_read_value<std::int16_t>(ptr);
		this->head.mac_style = ttf_read_value<std::uint16_t>(ptr);
		this->head.lowest_rec_ppem = ttf_read_value<std::uint16_t>(ptr);
		this->head.font_direction_hint = ttf_read_value<std::int16_t>(ptr);
		this->head.index_to_loc_format = ttf_read_value<std::int16_t>(ptr);
		this->head.glyph_data_format = ttf_read_value<std::int16_t>(ptr);

		// set canary to true, meaning we did indeed set the head table.
		this->head.canary = true;
	}

	void ttf::parse_maxp_table(std::string_view data, ttf_table table_descriptor)
	{
		tz::assert(data.size() > table_descriptor.offset + table_descriptor.length);
		data.remove_prefix(table_descriptor.offset);
		data.remove_suffix(data.size() - table_descriptor.length);
		tz::assert(data.size() == (table_descriptor.length));

		tz::assert(!this->maxp.canary, "When parsing maxp table, noticed canary already switched to true. Double maxp table discovery? Most likely malformed TTF.");

		const char* ptr = data.data();

		//this->head.major_version = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.version_fixed_point = ttf_read_value<std::int32_t>(ptr);
		this->maxp.version_fixed_point /= (1 << 16);
		this->maxp.num_glyphs = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_points = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_contours = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_composite_points = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_composite_contours = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_zones = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_twilight_points = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_storage = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_function_defs = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_instruction_defs = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_stack_elements = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_size_of_instructions = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_component_elements = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.max_component_depth = ttf_read_value<std::uint16_t>(ptr);
		this->maxp.canary = true;
	}

	void ttf::parse_hhea_table(std::string_view data, ttf_table table_descriptor)
	{
		tz::assert(data.size() > table_descriptor.offset + table_descriptor.length);
		data.remove_prefix(table_descriptor.offset);
		data.remove_suffix(data.size() - table_descriptor.length);
		tz::assert(data.size() == (table_descriptor.length));

		tz::assert(!this->hhea.canary, "When parsing hhea table, noticed canary already switched to true. Double hhea table discovery? Most likely malformed TTF.");

		const char* ptr = data.data();
		this->hhea.version_fixed_point = ttf_read_value<std::int32_t>(ptr);
		this->hhea.version_fixed_point /= (1 << 16);
		this->hhea.ascent = ttf_read_value<std::int16_t>(ptr);
		this->hhea.descent = ttf_read_value<std::int16_t>(ptr);
		this->hhea.line_gap = ttf_read_value<std::int16_t>(ptr);
		this->hhea.advance_width_max = ttf_read_value<std::uint16_t>(ptr);
		this->hhea.min_left_side_bearing = ttf_read_value<std::int16_t>(ptr);
		this->hhea.min_right_side_bearing = ttf_read_value<std::int16_t>(ptr);
		this->hhea.x_max_extent = ttf_read_value<std::int16_t>(ptr);
		this->hhea.caret_slope_rise = ttf_read_value<std::int16_t>(ptr);
		this->hhea.caret_slope_run = ttf_read_value<std::int16_t>(ptr);
		this->hhea.caret_offset = ttf_read_value<std::int16_t>(ptr);

		// skip 4 reserved places.
		ptr += sizeof(std::int16_t) * 4;

		this->hhea.metric_data_format = ttf_read_value<std::int16_t>(ptr);
		this->hhea.num_of_long_hor_metrics = ttf_read_value<std::uint16_t>(ptr);
		this->hhea.canary = true;
	}

	void ttf::parse_hmtx_table(std::string_view data, ttf_table table_descriptor)
	{
		tz::assert(data.size() > table_descriptor.offset + table_descriptor.length);
		data.remove_prefix(table_descriptor.offset);
		data.remove_suffix(data.size() - table_descriptor.length);
		tz::assert(data.size() == (table_descriptor.length));

		tz::assert(this->hhea.canary, "Cannot parse hmtx table until hhea table is parsed.");
		tz::assert(this->maxp.canary, "Cannot parse hmtx table until maxp table is parsed.");
		tz::assert(!this->hmtx.canary, "When parsing hmtx table, noticed canary already switched to true. Double hhea table discovery? Most likely malformed TTF.");

		const char* ptr = data.data();
		for(std::size_t i = 0; i < this->hhea.num_of_long_hor_metrics; i++)
		{
			this->hmtx.hmetrics.push_back
			({
				.advance_width = ttf_read_value<std::uint16_t>(ptr),
				.left_side_bearing = ttf_read_value<std::int16_t>(ptr)
			});
		}

		tz::assert(this->maxp.num_glyphs >= this->hhea.num_of_long_hor_metrics, "Not enough glyphs. Malformed TTF or logic error? Max glyphs = %u, Num long hor metrics = %u", static_cast<unsigned int>(this->maxp.num_glyphs), static_cast<unsigned int>(this->hhea.num_of_long_hor_metrics));
		for(std::size_t i = 0; std::cmp_less(i, this->maxp.num_glyphs - this->hhea.num_of_long_hor_metrics); i++)
		{
			this->hmtx.left_side_bearings.push_back(ttf_read_value<std::int16_t>(ptr));
		}
		this->hmtx.canary = true;
	}

	void ttf::parse_loca_table(std::string_view data, ttf_table table_descriptor)
	{
		tz::assert(data.size() > table_descriptor.offset + table_descriptor.length);
		data.remove_prefix(table_descriptor.offset);
		data.remove_suffix(data.size() - table_descriptor.length);
		tz::assert(data.size() == (table_descriptor.length));

		tz::assert(this->head.canary, "Cannot parse loca table until head table is parsed.");
		tz::assert(this->maxp.canary, "Cannot parse loca table until maxp table is parsed.");
		tz::assert(!this->loca.canary, "When parsing loca table, noticed canary already switched to true. Double hhea table discovery? Most likely malformed TTF.");
		const char* ptr = data.data();

		if(this->head.index_to_loc_format == 0)
		{
			// 16 bit
			for(std::size_t i = 0; std::cmp_less(i, this->maxp.num_glyphs); i++)
			{
				this->loca.locations16.push_back(ttf_read_value<std::uint16_t>(ptr));
			}
		}
		else
		{
			// 32 bit
			for(std::size_t i = 0; std::cmp_less(i, this->maxp.num_glyphs); i++)
			{
				this->loca.locations32.push_back(ttf_read_value<std::uint32_t>(ptr));
			}
		}
		this->loca.canary = true;
	}

	void ttf::parse_glyf_table(std::string_view data, ttf_table table_descriptor)
	{
		tz::assert(data.size() > table_descriptor.offset + table_descriptor.length);
		data.remove_prefix(table_descriptor.offset);
		data.remove_suffix(data.size() - table_descriptor.length);
		tz::assert(data.size() == (table_descriptor.length));

		tz::assert(this->head.canary, "Cannot parse glyf table until head table is parsed.");
		tz::assert(this->loca.canary, "Cannot parse glyf table until loca table is parsed.");
		tz::assert(!this->glyf.canary, "When parsing glyf table, noticed canary already switched to true. Double hhea table discovery? Most likely malformed TTF.");
		const char* ptr = data.data();
		const char* ptrcpy = ptr;

		if(this->head.index_to_loc_format == 0)
		{
			// locations16
			constexpr int multiplier = 2;
			for(std::uint16_t loc16 : this->loca.locations16)
			{
				auto loca_offset = loc16 * multiplier;
				ptr = ptrcpy + loca_offset;
				this->glyf.glyfs.push_back
				({
					.number_of_contours = ttf_read_value<std::int16_t>(ptr),
					.xmin = ttf_read_value<std::int16_t>(ptr),
					.ymin = ttf_read_value<std::int16_t>(ptr),
					.xmax = ttf_read_value<std::int16_t>(ptr),
					.ymax = ttf_read_value<std::int16_t>(ptr)
				});
			}
		}
		else
		{
			// locations32
			constexpr int multiplier = 1;
			for(std::uint32_t loc32 : this->loca.locations32)
			{
				auto loca_offset = loc32 * multiplier;
				ptr = ptrcpy + loca_offset;
				this->glyf.glyfs.push_back
				({
					.number_of_contours = ttf_read_value<std::int16_t>(ptr),
					.xmin = ttf_read_value<std::int16_t>(ptr),
					.ymin = ttf_read_value<std::int16_t>(ptr),
					.xmax = ttf_read_value<std::int16_t>(ptr),
					.ymax = ttf_read_value<std::int16_t>(ptr)
				});
			}
		}
		this->glyf.canary = true;
	}

	void ttf::parse_cmap_table(std::string_view data, ttf_table table_descriptor)
	{
		tz::assert(data.size() > table_descriptor.offset + table_descriptor.length);
		data.remove_prefix(table_descriptor.offset);
		data.remove_suffix(data.size() - table_descriptor.length);
		tz::assert(data.size() == (table_descriptor.length));

		//tz::assert(this->head.canary, "Cannot parse cmap table until head table is parsed.");
		tz::assert(!this->cmap.canary, "When parsing cmap table, noticed canary already switched to true. Double hhea table discovery? Most likely malformed TTF.");
		const char* ptr = data.data();

		this->cmap.version = ttf_read_value<std::uint16_t>(ptr);
		this->cmap.num_tables = ttf_read_value<std::uint16_t>(ptr);
		this->cmap.encoding_records.reserve(this->cmap.num_tables);
		for(std::size_t i = 0; i < this->cmap.num_tables; i++)
		{
			this->cmap.encoding_records.push_back
			({
				.platform_id = ttf_read_value<std::uint16_t>(ptr),
				.encoding_id = ttf_read_value<std::uint16_t>(ptr),
				.offset = ttf_read_value<std::uint32_t>(ptr)
			});
		}
		tz::assert(this->cmap.version == 0u, "TTF - Only cmap version 0 is supported. This font has cmap version %u", static_cast<unsigned int>(this->cmap.version));
		this->cmap.canary = true;
	}
}