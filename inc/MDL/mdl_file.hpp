#ifndef MDL_FILE_HPP
#define MDL_FILE_HPP
#include "file.hpp"
#include <map>

class MDLFile
{
public:
	explicit MDLFile(std::string file_path);
	explicit MDLFile(File raw_file = File());
	MDLFile(const MDLFile& copy) = default;
	MDLFile(MDLFile&& move) = default;
	~MDLFile() = default;
	MDLFile& operator=(const MDLFile& rhs) = default;
	
	const File& get_raw_file() const;
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
	File raw_file;
	std::map<std::string, std::string> parsed_tags;
	std::map<std::string, std::vector<std::string>> parsed_sequences;	
};

namespace mdl
{
	std::vector<std::string> read_lines(const std::string& filename);
	std::string read(const std::string& filename);
	namespace syntax
	{
		bool is_valid(const MDLFile& file);
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