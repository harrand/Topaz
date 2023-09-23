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
		char tag[5] = "DEAD";
		std::uint32_t checksum = 0u;
		std::uint32_t offset = 0u;
		std::uint32_t length = 0u;
	};

	struct ttf_head_table
	{
		std::uint16_t major_version = 0u;
		std::uint16_t minor_version = 0u;
		std::int32_t font_revision_fixed_point = 0;
		std::uint32_t checksum_adjustment = 0u;
		std::uint32_t magic = 0u;
		std::uint16_t flags = 0u;
		std::uint16_t units_per_em = 0u;
		std::uint64_t created_date = 0u;
		std::uint64_t modified_date = 0u;
		std::int16_t xmin = 0;
		std::int16_t ymin = 0;
		std::int16_t xmax = std::numeric_limits<std::int16_t>::max();
		std::int16_t ymax = std::numeric_limits<std::int16_t>::max();
		std::uint16_t mac_style = 0u;
		std::uint16_t lowest_rec_ppem = 0u;
		std::int16_t font_direction_hint = 0;
		std::int16_t index_to_loc_format = 0;
		std::int16_t glyph_data_format = 0;
		bool canary = false;
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
		void parse_head_table(std::string_view data, ttf_table table_descriptor);
		ttf_header header = {};

		std::vector<ttf_table> tables = {};
		ttf_head_table head = {};
	};
}

#endif //TOPAZ_IO_TTF_HPP