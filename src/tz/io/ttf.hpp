#ifndef TOPAZ_IO_TTF_HPP
#define TOPAZ_IO_TTF_HPP
#include <string_view>
#include <vector>

namespace tz::io
{
	struct ttf_header
	{
		std::uint32_t scalar_type = 0u;
		std::uint16_t num_tables = 0u;
		std::uint16_t search_range = 0u;
		std::uint16_t entry_selector = 0u;
		std::uint16_t range_shift = 0u;
	};

	struct ttf_table
	{
		char tag[4];
		std::uint32_t checksum = 0u;
		std::uint32_t offset = 0u;
		std::uint32_t length = 0u;
	};

	class ttf
	{
	public:
		ttf() = default;
		static ttf from_memory(std::string_view sv);
		static ttf from_file(const char* path);
		ttf(std::string_view ttf_data);
	private:
		std::string_view parse_header(std::string_view str);
		void parse_table_info(std::string_view str, std::string_view full_data);
		std::uint32_t calculate_table_checksum(std::string_view data, std::uint32_t offset, std::uint32_t length) const;
		ttf_header header = {};
		std::vector<ttf_table> tables = {};
	};
}

#endif //TOPAZ_IO_TTF_HPP