#ifndef MDL_HPP
#define MDL_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>

class RawFile
{
public:
	RawFile(std::string path);
	RawFile(const RawFile& copy) = default;
	RawFile(RawFile&& move) = default;
	RawFile& operator=(const RawFile& rhs) = default;
	
	const std::string& getPath() const;
	std::vector<std::string> getLines() const;
	std::string getData() const;
	void clear() const;
	void write(std::string data, bool clear) const;// will append the end of the file (so data is the final data in the file)
	void writeLine(std::string data, unsigned int line) const;
private:
	std::string path;
};

class MDLF
{
public:
	MDLF(RawFile rf);
	MDLF(const MDLF& copy) = default;
	MDLF(MDLF&& move) = default;
	MDLF& operator=(const MDLF& rhs) = default;
	
	const RawFile& getRawFile() const;
	void update() const;
	bool existsTag(const std::string& tag_name) const;
	bool existsSequence(const std::string& sequence_name) const;
	void addTag(std::string tag_name, std::string data) const;
	void addSequence(std::string sequence_name, std::vector<std::string> data) const;
	void deleteTag(std::string tag_name) const;
	void deleteSequence(std::string sequence_name) const;
	void editTag(std::string tag_name, std::string data) const;
	void editSequence(std::string sequence_name, std::vector<std::string> data) const;
	std::string getTag(const std::string& tag_name) const;
	std::vector<std::string> getSequence(const std::string& sequence_name) const;
	const std::map<std::string, std::string>& getParsedTags() const;
	const std::map<std::string, std::vector<std::string>>& getParsedSequences() const;
private:
	RawFile rf;
	mutable std::map<std::string, std::string> parsed_tags;
	mutable std::map<std::string, std::vector<std::string>> parsed_sequences;
	
	std::vector<std::string> splitString(std::string s, char d) const;
	std::string getTagName(std::string tag) const;
	bool hasEnding(const std::string& s, const std::string& e) const;
	bool hasBeginning(const std::string& s, const std::string& b) const;
	bool isSequence(const std::string& s) const;
	bool isEndOfSequence(const std::string& s) const;
	std::vector<std::string> getSequences(std::vector<std::string> lines, unsigned int index) const;
	
	void parse() const;
};

#endif