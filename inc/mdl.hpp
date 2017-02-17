#ifndef MDL_HPP
#define MDL_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
/*	MDL - Minimalist Data Language
	Written by Harry "Harrand" Hollands 2017. 
	Designed for use with C++11 or newer to manipulate and read data in a simple and efficient manner.
*/

class RawFile
{
public:
	RawFile(const std::string path);
	const std::string getPath() const;
	std::vector<std::string> getLines() const;
	std::string getLineByNumber(unsigned int index) const;
	std::string getData() const;
	void clear() const;
	void write(std::string data, bool clear) const;// will append the end of the file (so data is the final data in the file)
	void writeLine(std::string data, unsigned int line) const;
private:
	const std::string path;
};

class MDLF
{
public:
	MDLF(const RawFile rf);
	const RawFile getRawFile() const;
	void update();
	bool existsTag(std::string tagName) const;
	bool existsSequence(std::string sequenceName) const;
	void addTag(std::string tagName, std::string data) const;
	void addSequence(std::string sequenceName, std::vector<std::string> data) const;
	void deleteTag(std::string tagName) const;
	void deleteSequence(std::string sequenceName) const;
	std::string getTag(std::string tagName) const;
	std::vector<std::string> getSequence(std::string sequenceName) const;
	std::map<std::string, std::string> getParsedTags() const;
	std::map<std::string, std::vector<std::string>> getParsedSequences() const;
private:
	const RawFile rf;
	std::map<std::string, std::string> parsedTags;
	std::map<std::string, std::vector<std::string>> parsedSequences;
	
	std::vector<std::string> splitString(std::string s, char d) const;
	std::string getTagName(std::string tag) const;
	bool hasEnding(std::string s, std::string e) const;
	bool hasBeginning(std::string s, std::string b) const;
	bool isSequence(std::string s) const;
	bool isEndOfSequence(std::string s) const;
	std::vector<std::string> getSequences(std::vector<std::string> lines, unsigned int index) const;
	void parse();
};

#endif