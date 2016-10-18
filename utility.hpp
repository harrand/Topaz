#ifndef UTILITY_HPP
#define UTILITY_HPP
#include <string>
#include <cstdio>
#include <cmath>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

namespace StringUtility
{
	std::string toString(float obj);
	inline void toLower(std::string& data);
	inline void toUpper(std::string& data);
	bool beginsWith(std::string what, std::string withwhat);
	bool endsWith(std::string what, std::string withwhat);
	bool contains(std::string what, char withwhat);
	std::vector<std::string> splitString(std::string split, char delim);
	std::string replaceAllChar(std::string str, char toreplace, std::string replacewith);
	std::string substring(std::string str, unsigned int begin, unsigned int end);
}

class File
{
public:
	File(std::string filename);
	std::string getName();
	std::string getAll();
	std::vector<std::string> getLines();
	std::string getFromLine(unsigned int line);
	unsigned int lineCount();
	unsigned int wordCount();
	unsigned int charCount();
	bool exists();
	void clear();
	void write(std::string contents, bool cleardata);
	void writeLine(std::string contents, unsigned int line);
	void deleteFile();
private:
	std::string filename;
};

namespace FileUtility
{
	File createRelativeFile(std::string relativeFilePath);
	std::string getTag(File f, std::string tag);
	void setTag(File f, std::string tag, std::string value);
	void removeTag(File f, std::string tag);
}

namespace MathsUtility
{
	// c++98/03 use stringstreams to parse ints
	int parseTemplate(std::string x);
	float parseTemplateFloat(std::string x);
}

#endif //UTILITY_HPP