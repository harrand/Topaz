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



