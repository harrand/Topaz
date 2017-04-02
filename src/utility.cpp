#include "utility.hpp"

std::string StringUtility::toLower(std::string data)
{
	std::transform(data.begin(), data.end(), data.begin(), ::tolower);
	return data;
}

std::string StringUtility::toUpper(std::string data)
{
	std::transform(data.begin(), data.end(), data.begin(), ::toupper);
	return data;
}

bool StringUtility::beginsWith(const std::string& what, const std::string& withwhat)
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

bool StringUtility::endsWith(const std::string& what, const std::string& withwhat)
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

bool StringUtility::contains(const std::string& what, char withwhat)
{
	const char* whatcstr = what.c_str();
	for(unsigned int i = 0; i < what.length(); i++)
		if(whatcstr[i] == withwhat)
			return true;
}

std::vector<std::string> StringUtility::splitString(const std::string& split, char delim)
{
	unsigned int strsize = split.length();
	const char* str = split.c_str();
	std::vector<std::string> ret;
	unsigned int numcount = 0;
	std::unordered_map<unsigned int, unsigned int> num_pos;
	for(unsigned int i = 0; i < strsize; i++)
	{
		char cur = str[i];
		if(cur == delim)
		{
			num_pos[numcount] = i;
			numcount++;
		}
		if(i == (strsize-1))
		{
			for(unsigned int j = 0; j < numcount; j++)
			{
				unsigned int prevpos;
				if(j == 0)
					prevpos = 0;
				unsigned int posdelim = num_pos[j];
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
					prevpos+=(posdelim-prevpos);
					ret.push_back(split.substr((prevpos+1), (strsize-prevpos)));
				}
				prevpos = posdelim;
			}
			
		}
	}
	return ret;
}

std::string StringUtility::replaceAllChar(const std::string& str, char toreplace, const std::string& replacewith)
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

std::string StringUtility::substring(const std::string& str, unsigned int begin, unsigned int end)
{
	unsigned int strsize = str.length();
	if(end > strsize)
		return "_";
	return str.substr((begin-1), (end-begin)+1);
}

Force::Force(Vector3F size): size(size){}

Vector3F Force::getSize() const
{
	return this->size;
}

void Force::setSize(Vector3F size)
{
	this->size = size;
}

Force Force::operator+(const Force& other) const
{
	return Force(this->size + other.getSize());
}

Force Force::operator-(const Force& other) const
{
	return Force(this->size - other.getSize());
}

Force Force::operator*(float rhs) const
{
	return Force(this->size * rhs);
}

Force Force::operator/(float rhs) const
{
	return Force(this->size / rhs);
}

void Force::operator+=(const Force& other)
{
	this->size += other.getSize();
}

void Force::operator-=(const Force& other)
{
	this->size -= other.getSize();
}



