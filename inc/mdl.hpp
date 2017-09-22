#ifndef MDL_HPP
#define MDL_HPP
#include <cstddef>
#include <string>
#include <vector>
#include <map>

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
	
	const std::string& getPath() const;
	std::vector<std::string> getLines() const;
	std::string getData() const;
	void clear() const;
	void write(std::string data, bool clear) const;
	void writeLine(std::string data, std::size_t line) const;
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
	
	const RawFile& getRawFile() const;
	bool existsTag(const std::string& tag_name) const;
	bool existsSequence(const std::string& sequence_name) const;
	void addTag(std::string tag_name, std::string data);
	void addSequence(std::string sequence_name, std::vector<std::string> data);
	void deleteTag(std::string tag_name);
	void deleteSequence(std::string sequence_name);
	void editTag(std::string tag_name, std::string data);
	void editSequence(std::string sequence_name, std::vector<std::string> data);
	std::string getTag(const std::string& tag_name) const;
	std::vector<std::string> getSequence(const std::string& sequence_name) const;
	const std::map<std::string, std::string>& getParsedTags() const;
	const std::map<std::string, std::vector<std::string>>& getParsedSequences() const;
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
		bool isValid(const MDLF& file);
		bool isComment(const std::string& line);
		bool isTag(const std::string& line);
		bool isSequence(const std::string& line);
		bool isEndOfSequence(const std::string& line);
	}
	namespace util
	{
		std::vector<std::string> splitString(const std::string& string, const std::string& delimiter);
		bool endsWith(const std::string& string, const std::string& suffix);
		bool beginsWith(const std::string& string, const std::string& prefix);
		std::string findTagName(const std::string& line);
		std::string findTagValue(const std::string& line);
		std::string findSequenceName(const std::string& line);
		std::vector<std::string> findSequenceValues(const std::vector<std::string>& lines, std::size_t index);
	}
}

#endif