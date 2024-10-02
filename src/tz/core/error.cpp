#include "tz/core/error.hpp"

namespace tz
{
	std::string last_error_str = "";

	std::string_view last_error()
	{
		return last_error_str;
	}

	void set_last_error(std::string errmsg)
	{
		last_error_str = errmsg;
	}
}