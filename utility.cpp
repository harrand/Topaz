#include "utility.hpp"

std::string StringUtility::toString(float obj)
{
	std::ostringstream ostr;
	ostr << obj;
	return ostr.str();
}

inline void StringUtility::toLower(std::string& data)
{
	std::transform(data.begin(), data.end(), data.begin(), ::tolower);
}

inline void StringUtility::toUpper(std::string& data)
{
	std::transform(data.begin(), data.end(), data.begin(), ::toupper);
}

bool StringUtility::beginsWith(std::string what, std::string withwhat)
{
	unsigned int strsize = what.length(), withsize = withwhat.length();
	const char* str = what.c_str();
	const char* with = withwhat.c_str();
	if(strsize < withsize || str[0] != with[0])
		return false;
	for(unsigned int iter = 0; iter < withsize; iter++)
	{
		char str_at = str[iter], with_at = with[iter];
		if(str_at == with_at)
		{
			if(iter == withsize)
				return true;
		}
		else
		{
			return false;
		}
	}
}

bool StringUtility::endsWith(std::string what, std::string withwhat)
{
	unsigned int strsize = what.length(), withsize = withwhat.length();
	const char* str = what.c_str();
	const char* with = withwhat.c_str();
	if(strsize < withsize || str[strsize-withsize] != with[0])
		return false;
	for(unsigned int iter = 0; iter < withsize; iter++)
	{
		char str_at = str[strsize-(withsize-iter)], with_at = with[iter];
		if(str_at == with_at)
		{
			if(iter == withsize)
				return true;
		}
		else
		{
			return false;
		}
	}
}

bool StringUtility::contains(std::string what, char withwhat)
{
	const char* whatcstr = what.c_str();
	for(unsigned int i = 0; i < what.length(); i++)
		if(whatcstr[i] == withwhat)
			return true;
}

std::vector<std::string> StringUtility::splitString(std::string split, char delim)
{
	unsigned int strsize = split.length();
	const char* str = split.c_str();
	std::vector<std::string> ret;
	unsigned int numcount = 0;
	std::map<unsigned int, unsigned int> num_pos;
	for(unsigned int i = 0; i < strsize; i++)
	{
		char cur = str[i];
		//TEMP
		if(cur == delim)
		{
			num_pos[numcount] = i;
			numcount++;
		}
		if(i == (strsize-1))
		{
			// at the end, sort everything out
			for(unsigned int j = 0; j < numcount; j++)
			{
				// go through every bit of the map where there's a delim, and then form substrings
				unsigned int prevpos;
				if(j == 0)
					prevpos = 0;
				unsigned int posdelim = num_pos[j];
				//if(prevpos != 0)
				//{
				if(prevpos == 0)
				{
					std::string sub = split.substr(prevpos, (posdelim-prevpos));
					ret.push_back(sub);
				}
				else
				{
					std::string sub = split.substr((prevpos+1), (posdelim-prevpos-1));
					ret.push_back(sub);
				}
				if(j == (numcount-1))
				{
					//right at the end, we add on the last bit because it probably doesnt end with a space (it shouldnt)
					prevpos+=(posdelim-prevpos);
					ret.push_back(split.substr((prevpos+1), (strsize-prevpos)));
				}
				//}
				prevpos = posdelim;
			}
			
		}
	}
	return ret;
}

std::string StringUtility::replaceAllChar(std::string str, char toreplace, std::string replacewith)
{
	std::string res;
	std::vector<std::string> splitdelim = StringUtility::splitString(str, toreplace);
	for(unsigned int i = 0; i < splitdelim.size(); i++)
	{
		res += splitdelim.at(i);
		res += replacewith;
	}
	return res;
}

std::string StringUtility::substring(std::string str, unsigned int begin, unsigned int end)
{
	unsigned int strsize = str.length();
	if(end > strsize)
		return "_";
	return str.substr((begin-1), (end-begin)+1);
}

File::File(std::string filename)
{
	this->filename = filename;
}

std::string File::getName()
{
	return this->filename;
}

std::string File::getAll()
{
	std::ifstream in((this->filename).c_str());
	std::string str, res;
	while(std::getline(in, str))
	{
		res += str;
	}
	return res;
}

std::vector<std::string> File::getLines()
{
	std::vector<std::string> res;
	std::string str;
	std::ifstream in((this->filename).c_str());
	while(std::getline(in, str, '\n'))
	{
		res.push_back(str);
	}
	return res;
}

std::string File::getFromLine(unsigned int line)
{
	return this->getLines().at(line);
}

unsigned int File::lineCount()
{
	return (this->getLines()).size();
}

unsigned int File::wordCount()
{
	std::vector<std::string> lines = this->getLines();
	unsigned int res = 0;
	for(unsigned int i = 0; i < lines.size(); i++)
	{
		std::string l = lines.at(i);
		std::vector<std::string> wordsinline = StringUtility::splitString(l, ' ');
		res += wordsinline.size();
	}
	return res;	
}

unsigned int File::charCount()
{
	std::vector<std::string> lines = this->getLines();
	unsigned int chars = 0;
	for(unsigned int i = 0; i < lines.size(); i++)
	{
		std::string l = lines.at(i);
		std::vector<std::string> wordsinline = StringUtility::splitString(l, ' ');
		//res += wordsinline.size();
		for(unsigned int j = 0; j < wordsinline.size(); j++)
		{
			chars += (wordsinline.at(j)).length();
		}
	}
	return chars;
}

bool File::exists()
{
    std::ifstream infile((this->filename).c_str());
    return infile.good();
}

void File::clear()
{
	std::ofstream out;
	out.open((this->filename).c_str(), std::ofstream::out | std::ofstream::trunc);
	out.close();
}

void File::write(std::string contents, bool cleardata)
{
	std::vector<std::string> lines = this->getLines();
	std::ofstream out;
	out.open((this->filename).c_str(), std::ofstream::out | std::ofstream::trunc);
	std::vector<std::string> outdata;
	if(!cleardata)
	{
		for(unsigned int i = 0; i < lines.size(); i++)
		{
			outdata.push_back(lines.at(i));
		}
	}
	outdata.push_back(contents);
	for(unsigned int j = 0; j < outdata.size(); j++)
	{
		std::string lo = outdata.at(j);
		lo += "\n";
		const char* locstr = lo.c_str();
		out << locstr;
	}
}

void File::writeLine(std::string contents, unsigned int line)
{
	std::vector<std::string> lines = this->getLines();
	lines.at(line) = contents;
	this->clear();
	for(unsigned int i = 0; i < lines.size(); i++)
	{
		std::string l = lines.at(i);
		this->write(l, false);
	}
}

void File::deleteFile()
{
	std::remove((this->filename).c_str());
}

File FileUtility::createRelativeFile(std::string relativeFilePath)
{
	std::ofstream autocreate(relativeFilePath.c_str());
	autocreate.close();
	return File(relativeFilePath);
}

std::string FileUtility::getTag(File f, std::string tag)
{
	// make both lower case to make it case insensitive
	std::string res = "_";
	StringUtility::toLower(tag);
	for(unsigned int i = 0; i < f.lineCount(); i++)
	{
		std::string s = f.getFromLine(i);
		StringUtility::toLower(s);
		if(StringUtility::beginsWith(s, tag + ": "))
		{
			// add 3. 1 because we dont want to count from 0. 1 because of a colon and another because of a space
			res = StringUtility::substring(s, (tag.length() + 3), s.length());
		}
	}
	return res;
}

void FileUtility::setTag(File f, std::string tag, std::string value)
{
	std::vector<std::string> lines = f.getLines();
	unsigned int existline;
	bool exists = false;
	for(unsigned int i = 0; i < f.lineCount(); i++)
	{
		std::string l = f.getFromLine(i);
		if(StringUtility::beginsWith(l, tag))
		{
			exists = true;
			existline = i;
		}
	}
	if(!exists)
	{
		// easy mode, just write the tag at the bottom of the file
		f.write(tag + ": " + value, false);
	}
	else
	{
		std::string curval = FileUtility::getTag(f, tag);
		f.writeLine(tag + ": " + value, existline);
		//std::cout << "[FileUtility]: CAUTION: Tag " << tag << ": <" << curval << "> has been overwritten by <" << value << ">!\n";
	}
}

void FileUtility::removeTag(File f, std::string tag)
{
	if(FileUtility::getTag(f, tag) == "_")
		return;
	// tag definitely exists. we need to find the line on which it exists
	StringUtility::toLower(tag);
	for(unsigned int i = 0; i < f.lineCount(); i++)
	{
		std::string s = f.getFromLine(i);
		StringUtility::toLower(s);
		if(StringUtility::beginsWith(s, tag + ": "))
		{
			// tag exists on this line, remove it.
			f.writeLine("", i);
		}
	}
}

int MathsUtility::parseTemplate(std::string x)
{
	std::istringstream is(x);
	int i;
	is >> i;
	return i;
}

float MathsUtility::parseTemplateFloat(std::string x)
{
	//Ensure theres no spaces or extra shit in the string
	std::string y = StringUtility::replaceAllChar(x, ' ', "");
	std::istringstream is(x);
	float i;
	is >> i;
	return i;
}



