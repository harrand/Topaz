#ifndef TZ_GL_API_SCHEDULE_HPP
#define TZ_GL_API_SCHEDULE_HPP
#include "tz/core/data/handle.hpp"
#include <vector>

namespace tz::gl
{
	using eid_t = unsigned int;
	constexpr eid_t nullevt = std::numeric_limits<eid_t>::max();
	struct event
	{
		eid_t eid = nullevt;
		std::vector<eid_t> dependencies = {};
	};

	struct schedule
	{
		std::vector<event> events = {};
	};

}

#endif // TZ_GL_API_SCHEDULE_HPP
