#ifndef FILE_HPP
#define FILE_HPP
#include <string>
#include <vector>
#include <string_view>
#include <cstddef>

namespace mdl
{
	constexpr char default_string[] = "0";
}

class File
{
public:
	explicit File(std::string path);
	File();
	File(const File& copy) = default;
	File(File&& move) = default;
	~File() = default;
	File& operator=(const File& rhs) = default;
	
	std::string_view get_path() const;
	std::vector<std::string> get_lines() const;
	std::string get_data() const;
	void clear() const;
	void write(std::string data, bool clear) const;
	void write_line(std::string data, std::size_t line) const;
private:
	std::string path;
};

#endif