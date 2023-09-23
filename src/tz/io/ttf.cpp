#include "tz/io/ttf.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
#include "tz/core/endian.hpp"
#include <fstream>
#include <cstdio>
#include <cstring>

namespace tz::io
{
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

			if(tbl.tag[0] != 'h' || tbl.tag[1] != 'e' || tbl.tag[2] != 'a' || tbl.tag[3] != 'd')
			{
				std::uint32_t calc = calculate_table_checksum(full_data, tbl.offset, tbl.length);
				tz::assert(calc == tbl.checksum);
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
}