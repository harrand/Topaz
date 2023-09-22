#include "tz/io/ttf.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
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

	std::uint32_t swap_endian(std::uint32_t value) {
		return ((value >> 24) & 0x000000FF) |  // Move the first byte to the last position
			((value >> 8)  & 0x0000FF00) |  // Move the second byte to the second position
			((value << 8)  & 0x00FF0000) |  // Move the third byte to the third position
			((value << 24) & 0xFF000000);   // Move the last byte to the first position
	}

	std::uint16_t swap_endian(std::uint16_t value) {
		return ((value >> 8) & 0x00FF) |      // Swap the high byte and low byte
			((value << 8) & 0xFF00);
	}

	std::string_view ttf::parse_header(std::string_view str)
	{
		const char* ptr = str.data();
		this->header.scalar_type = swap_endian(*reinterpret_cast<const std::uint32_t*>(ptr));	
		ptr += sizeof(std::uint32_t);
		this->header.num_tables = swap_endian(*reinterpret_cast<const std::uint16_t*>(ptr));	
		ptr += sizeof(std::uint16_t);
		this->header.search_range = swap_endian(*reinterpret_cast<const std::uint16_t*>(ptr));
		ptr += sizeof(std::uint16_t);
		this->header.entry_selector = swap_endian(*reinterpret_cast<const std::uint16_t*>(ptr));
		ptr += sizeof(std::uint16_t);
		this->header.range_shift = swap_endian(*reinterpret_cast<const std::uint16_t*>(ptr));
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
			tbl.checksum = swap_endian(*reinterpret_cast<const std::uint32_t*>(ptr));
			ptr += sizeof(std::uint32_t);
			tbl.offset = swap_endian(*reinterpret_cast<const std::uint32_t*>(ptr));
			ptr += sizeof(std::uint32_t);
			tbl.length = swap_endian(*reinterpret_cast<const std::uint32_t*>(ptr));
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
			sum += swap_endian(*reinterpret_cast<const std::uint32_t*>(data.data() + offset + i));
		}
		return sum;
	}
}