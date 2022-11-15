#ifndef TZ_HPP
#define TZ_HPP
#include "hdk/data/version.hpp"

namespace tz
{
	void initialise();
	void terminate();
	hdk::version get_version();
}

#endif // TZ_HPP
