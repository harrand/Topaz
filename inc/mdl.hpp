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
	bool existsTag(std::string tagName) const;
	bool existsSequence(std::string sequenceName) const;
	void addTag(std::string tagName, std::string data) const;
	void addSequence(std::string sequenceName, std::vector<std::string> data) const;
	void deleteTag(std::string tagName) const;
	void deleteSequence(std::string sequenceName) const;
	void editTag(std::string tagName, std::string data) const;
	void editSequence(std::string sequenceName, std::vector<std::string> data) const;
	std::string getTag(std::string tagName) const;
	std::vector<std::string> getSequence(std::string sequenceName) const;
	std::map<std::string, std::string> getParsedTags() const;
	std::map<std::string, std::vector<std::string>> getParsedSequences() const;
private:
	RawFile rf;
	mutable std::map<std::string, std::string> parsedTags;
	mutable std::map<std::string, std::vector<std::string>> parsedSequences;
	
	std::vector<std::string> splitString(std::string s, char d) const;
	std::string getTagName(std::string tag) const;
	bool hasEnding(std::string s, std::string e) const;
	bool hasBeginning(std::string s, std::string b) const;
	bool isSequence(std::string s) const;
	bool isEndOfSequence(std::string s) const;
	std::vector<std::string> getSequences(std::vector<std::string> lines, unsigned int index) const;
	
	void parse() const;
};

#endif