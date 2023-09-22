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
		this->parse_header(ttf_data);
		this->parse_table_info(ttf_data);
	}

	void ttf::parse_header(std::string_view& str)
	{
		const char* ptr = str.data();
		this->header.scalar_type = *reinterpret_cast<const std::uint32_t*>(ptr);	
		ptr += sizeof(std::uint32_t);
		this->header.num_tables = *reinterpret_cast<const std::uint16_t*>(ptr);	
		ptr += sizeof(std::uint16_t);
		this->header.search_range = *reinterpret_cast<const std::uint16_t*>(ptr);
		ptr += sizeof(std::uint16_t);
		this->header.entry_selector = *reinterpret_cast<const std::uint16_t*>(ptr);
		ptr += sizeof(std::uint16_t);
		this->header.range_shift = *reinterpret_cast<const std::uint16_t*>(ptr);
		ptr += sizeof(std::uint16_t);

		auto byte_diff = std::distance(str.data(), ptr);
		tz::assert(std::cmp_greater(str.size(), byte_diff));
		str.remove_prefix(byte_diff);
	}

	void ttf::parse_table_info(std::string_view str)
	{
		const char* ptr = str.data();
		tz::assert(this->tables.empty());
		for(std::size_t i = 0; i < this->header.num_tables; i++)
		{
			ttf_table& tbl = this->tables.emplace_back();
			std::memcpy(tbl.tag, ptr, 4);
			ptr += 4;	
			tbl.checksum = *reinterpret_cast<const std::uint32_t*>(ptr);
			ptr += sizeof(std::uint32_t);
			tbl.offset = *reinterpret_cast<const std::uint32_t*>(ptr);
			ptr += sizeof(std::uint32_t);
			tbl.length = *reinterpret_cast<const std::uint32_t*>(ptr);
			ptr += sizeof(std::uint32_t);
		}
	}
}