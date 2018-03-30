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

/**
 * Readable and writable File.
 */
class File
{
public:
	/**
	 * Construct a new File object. No writing/reading occurs until a write or reading method is invoked.
	 * This means that even if 'path' does not point to a valid File, it shall not be created.
	 * @param path : Absolute path of the desired File.
	 */
	explicit File(std::string path);
	File(const File& copy) = default;
	File(File&& move) = default;
	virtual ~File() = default;
	File& operator=(const File& rhs) = default;

	/**
	 * Retrieve the absolute path of the File.
	 */
	std::string get_path() const;
	/**
	 * Find out if the File currently exists in the file-system.
	 */
	bool exists() const;
	/**
	 * Return a vector, where each element is a line in the File.
	 */
	std::vector<std::string> get_lines() const;
	/**
	 * Slurp the data in the file straight into a string object.
	 */
	std::string get_data() const;
	/**
	 * Clears the File of all its data. If the File does not exist, it shall be created.
	 */
	void clear() const;
	void write(std::string data, bool clear) const;
	void write_line(std::string data, std::size_t line) const;
private:
	std::string path;
};
#endif