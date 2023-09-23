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
	T ttf_read_value(const char* data)
	{
		return tz::big_endian(*reinterpret_cast<const T*>(data));
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
		// by the time we've parsed tables, we expect all the tables to be sorted.
		// let's do a sanity check ensuring that the canary of the head table has been set to true.
		tz::assert(this->head.canary, "TTF Head Table canary value was never set to true, this means that a head table was not located. Most likely the TTF is malformed or corrupted.");
	}

	std::string_view ttf::parse_header(std::string_view str)
	{
		const char* ptr = str.data();
		this->header.scalar_type = tz::big_endian(*reinterpret_cast<const std::uint32_t*>(ptr));	
		ptr += sizeof(std::uint32_t);
		this->header.num_tables = tz::big_endian(*reinterpret_cast<const std::uint16_t*>(ptr));	
		ptr += sizeof(std::uint16_t);
		this->header.search_range = tz::big_endian(*reinterpret_cast<const std::uint16_t*>(ptr));
		ptr += sizeof(std::uint16_t);
		this->header.entry_selector = tz::big_endian(*reinterpret_cast<const std::uint16_t*>(ptr));
		ptr += sizeof(std::uint16_t);
		this->header.range_shift = tz::big_endian(*reinterpret_cast<const std::uint16_t*>(ptr));
		ptr += sizeof(std::uint16_t);

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
			tbl.checksum = tz::big_endian(*reinterpret_cast<const std::uint32_t*>(ptr));
			ptr += sizeof(std::uint32_t);
			tbl.offset = tz::big_endian(*reinterpret_cast<const std::uint32_t*>(ptr));
			ptr += sizeof(std::uint32_t);
			tbl.length = tz::big_endian(*reinterpret_cast<const std::uint32_t*>(ptr));
			ptr += sizeof(std::uint32_t);

			if(std::string{tbl.tag} == "head")
			{
				this->parse_head_table(full_data, tbl);
			}
			else
			{
				std::uint32_t calc = calculate_table_checksum(full_data, tbl.offset, tbl.length);
				tz::assert(calc == tbl.checksum);

				// parse other table types...
			}
		}
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
		ptr += sizeof(this->head.major_version);

		this->head.minor_version = ttf_read_value<std::uint16_t>(ptr);
		ptr += sizeof(this->head.minor_version);

		this->head.font_revision_fixed_point = ttf_read_value<std::int32_t>(ptr);
		// fixed-point conversion: divide by (1 >> 16)
		this->head.font_revision_fixed_point /= (1 << 16);
		ptr += sizeof(this->head.font_revision_fixed_point);

		this->head.checksum_adjustment = ttf_read_value<std::uint32_t>(ptr);
		ptr += sizeof(this->head.checksum_adjustment);

		this->head.magic = ttf_read_value<std::uint32_t>(ptr);
		ptr += sizeof(this->head.magic);

		this->head.flags = ttf_read_value<std::uint16_t>(ptr);
		ptr += sizeof(this->head.flags);

		this->head.units_per_em = ttf_read_value<std::uint16_t>(ptr);
		ptr += sizeof(this->head.units_per_em);

		// ignore created and modified date for now. aids.
		ptr += sizeof(std::uint64_t) * 2;

		this->head.xmin = ttf_read_value<std::int16_t>(ptr);
		ptr += sizeof(this->head.xmin);

		this->head.ymin = ttf_read_value<std::int16_t>(ptr);
		ptr += sizeof(this->head.ymin);

		this->head.xmax = ttf_read_value<std::int16_t>(ptr);
		ptr += sizeof(this->head.xmax);

		this->head.ymax = ttf_read_value<std::int16_t>(ptr);
		ptr += sizeof(this->head.ymax);

		this->head.mac_style = ttf_read_value<std::uint16_t>(ptr);
		ptr += sizeof(this->head.mac_style);

		this->head.lowest_rec_ppem = ttf_read_value<std::uint16_t>(ptr);
		ptr += sizeof(this->head.lowest_rec_ppem);

		this->head.font_direction_hint = ttf_read_value<std::int16_t>(ptr);
		ptr += sizeof(this->head.font_direction_hint);

		this->head.index_to_loc_format = ttf_read_value<std::int16_t>(ptr);
		ptr += sizeof(this->head.index_to_loc_format);

		this->head.glyph_data_format = ttf_read_value<std::int16_t>(ptr);
		ptr += sizeof(this->head.glyph_data_format);

		// set canary to true, meaning we did indeed set the head table.
		this->head.canary = true;
	}
}