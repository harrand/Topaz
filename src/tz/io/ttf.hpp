#ifndef TOPAZ_IO_TTF_HPP
#define TOPAZ_IO_TTF_HPP
#include "tz/io/image.hpp"
#include "tz/core/data/vector.hpp"
#include <string_view>
#include <vector>
#include <limits>
#include <unordered_map>

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

	struct ttf_maxp_table
	{
		std::int32_t version_fixed_point = 0;
		std::uint16_t num_glyphs = 0u;
		std::uint16_t max_points = 0u;
		std::uint16_t max_contours = 0u;
		std::uint16_t max_composite_points = 0u;
		std::uint16_t max_composite_contours = 0u;
		std::uint16_t max_zones = 0u;
		std::uint16_t max_twilight_points = 0u;
		std::uint16_t max_storage = 0u;
		std::uint16_t max_function_defs = 0u;
		std::uint16_t max_instruction_defs = 0u;
		std::uint16_t max_stack_elements = 0u;
		std::uint16_t max_size_of_instructions = 0u;
		std::uint16_t max_component_elements = 0u;
		std::uint16_t max_component_depth = 0u;
		bool canary = false;
	};

	struct ttf_hhea_table
	{
		std::int32_t version_fixed_point = 0;
		std::int16_t ascent = 0;
		std::int16_t descent = 0;
		std::int16_t line_gap = 0;
		std::uint16_t advance_width_max = 0u;
		std::int16_t min_left_side_bearing = 0;
		std::int16_t min_right_side_bearing = 0;
		std::int16_t x_max_extent = 0;
		std::int16_t caret_slope_rise = 0;
		std::int16_t caret_slope_run = 0;
		std::int16_t caret_offset = 0;

		std::int16_t metric_data_format = 0;
		std::uint16_t num_of_long_hor_metrics = 0u;
		bool canary = false;
	};

	struct ttf_hmtx_table
	{
		struct hmetrics_t
		{
			std::uint16_t advance_width = 0u;
			std::int16_t left_side_bearing = 0;
		};

		std::vector<hmetrics_t> hmetrics = {};
		std::vector<std::int16_t> left_side_bearings = {};
		bool canary = false;
	};

	struct ttf_loca_table
	{
		std::vector<std::uint16_t> locations16 = {};
		std::vector<std::uint32_t> locations32 = {};
		bool canary = false;
	};

	struct ttf_glyf_elem
	{
		std::int16_t number_of_contours = 0;
		std::int16_t xmin = 0;
		std::int16_t ymin = 0;
		std::int16_t xmax = 0;
		std::int16_t ymax = 0;

		std::vector<std::byte> instructions = {};
		std::vector<std::byte> flags = {};
		std::vector<int> x_coords = {};
		std::vector<int> y_coords = {};
		std::vector<std::uint16_t> end_pts_of_contours = {};
	};

	struct ttf_glyf_table
	{
		std::vector<ttf_glyf_elem> glyfs = {};
		bool canary = false;
	};

	struct ttf_cmap_encoding_record
	{
		std::uint16_t platform_id = 0u;
		std::uint16_t encoding_id = 0u;
		std::uint32_t offset = 0u;
	};

	struct ttf_cmap_table
	{
		std::uint16_t version = 0u;
		std::uint16_t num_tables = 0u;
		std::vector<ttf_cmap_encoding_record> encoding_records = {};
		std::unordered_map<std::uint16_t, std::int16_t> glyph_index_map = {};
		bool canary = false;
	};

	struct ttf_glyph_spacing_info
	{
		tz::vec2i position = tz::vec2ui::zero();
		tz::vec2ui dimensions = tz::vec2ui::zero();
		int left_side_bearing = 0u;
		int right_side_bearing = 0u;
	};

	struct ttf_glyph_contour
	{
		std::vector<std::pair<tz::vec2, tz::vec2>> edges = {};
	};

	struct ttf_glyph_shape_info
	{
		std::vector<ttf_glyph_contour> contours = {};
	};

	struct ttf_glyph
	{
		ttf_glyph_spacing_info spacing = {};
		ttf_glyph_shape_info shape = {};
	};
	
	constexpr char ttf_alphabet[] = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

	struct ttf_glyph_map : public std::unordered_map<char, ttf_glyph>
	{
		ttf_glyph_map() = default;
	};

	class ttf
	{
	public:
		ttf() = default;
		static ttf from_memory(std::string_view sv);
		static ttf from_file(const char* path);

		struct rasterise_info
		{
			tz::vec2ui dimensions;
			float angle_threshold;
			float range;
			float scale;
			tz::vec2 translate;
		};

		tz::io::image rasterise_msdf(char c, rasterise_info i) const;
		const ttf_glyph_map& get_glyphs() const;
		ttf(std::string_view ttf_data);
	private:
		std::string_view parse_header(std::string_view str);
		void parse_table_info(std::string_view str, std::string_view full_data);
		ttf_table find_table_by_tag(const char* tag) const;
		void parse_tables(std::string_view data);
		std::uint32_t calculate_table_checksum(std::string_view data, std::uint32_t offset, std::uint32_t length) const;
		void parse_head_table(std::string_view data, ttf_table table_descriptor);
		void parse_maxp_table(std::string_view data, ttf_table table_descriptor);
		void parse_hhea_table(std::string_view data, ttf_table table_descriptor);
		void parse_hmtx_table(std::string_view data, ttf_table table_descriptor);
		void parse_loca_table(std::string_view data, ttf_table table_descriptor);
		void parse_glyf_table(std::string_view data, ttf_table table_descriptor);
		void parse_cmap_table(std::string_view data, ttf_table table_descriptor);
		void populate_glyph_map();
		ttf_header header = {};

		std::vector<ttf_table> tables = {};
		ttf_head_table head = {};
		ttf_maxp_table maxp = {};
		ttf_hhea_table hhea = {};
		ttf_hmtx_table hmtx = {};
		ttf_loca_table loca = {};
		ttf_glyf_table glyf = {};
		ttf_cmap_table cmap = {};

		ttf_glyph_map glyphs = {};
	};
}

#endif //TOPAZ_IO_TTF_HPP