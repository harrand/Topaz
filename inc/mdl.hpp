#ifndef MDL_HPP
#define MDL_HPP
#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <string_view>

namespace mdl
{
	constexpr char default_string[] = "0";
}

class RawFile
{
public:
	RawFile(std::string path = mdl::default_string);
	RawFile(const RawFile& copy) = default;
	RawFile(RawFile&& move) = default;
	~RawFile() = default;
	RawFile& operator=(const RawFile& rhs) = default;
	
	std::string_view get_path() const;
	std::vector<std::string> get_lines() const;
	std::string get_data() const;
	void clear() const;
	void write(std::string data, bool clear) const;
	void write_line(std::string data, std::size_t line) const;
private:
	std::string path;
};

class MDLF
{
public:
	MDLF(std::string file_path = mdl::default_string);
	MDLF(RawFile raw_file);
	MDLF(const MDLF& copy) = default;
	MDLF(MDLF&& move) = default;
	~MDLF() = default;
	MDLF& operator=(const MDLF& rhs) = default;
	
	const RawFile& get_raw_file() const;
	bool exists_tag(const std::string& tag_name) const;
	bool exists_sequence(const std::string& sequence_name) const;
	void add_tag(std::string tag_name, std::string data);
	void add_sequence(std::string sequence_name, std::vector<std::string> data);
	void delete_tag(std::string tag_name);
	void delete_sequence(std::string sequence_name);
	void edit_tag(std::string tag_name, std::string data);
	void edit_sequence(std::string sequence_name, std::vector<std::string> data);
	std::string get_tag(const std::string& tag_name) const;
	std::vector<std::string> get_sequence(const std::string& sequence_name) const;
	const std::map<std::string, std::string>& get_parsed_tags() const;
	const std::map<std::string, std::vector<std::string>>& get_parsed_sequences() const;
	void update();
private:
	RawFile raw_file;
	std::map<std::string, std::string> parsed_tags;
	std::map<std::string, std::vector<std::string>> parsed_sequences;	
};

namespace mdl
{
	namespace syntax
	{
		bool is_valid(const MDLF& file);
		bool is_comment(const std::string& line);
		bool is_tag(const std::string& line);
		bool is_sequence(const std::string& line);
		bool is_end_of_sequence(const std::string& line);
	}
	namespace util
	{
		std::vector<std::string> split_string(const std::string& string, const std::string& delimiter);
		bool ends_with(const std::string& string, const std::string& suffix);
		bool begins_with(const std::string& string, const std::string& prefix);
		std::string find_tag_name(const std::string& line);
		std::string find_tag_value(const std::string& line);
		std::string find_sequence_name(const std::string& line);
		std::vector<std::string> find_sequence_values(const std::vector<std::string>& lines, std::size_t index);
	}
}

#endif