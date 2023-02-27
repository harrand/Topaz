#ifndef TZ_GL_API_SCHEDULE_HPP
#define TZ_GL_API_SCHEDULE_HPP
#include "tz/core/data/handle.hpp"
#include <vector>
#include <span>
#include <unordered_set>

namespace tz::gl
{
	using eid_t = unsigned int;
	constexpr eid_t nullevt = std::numeric_limits<eid_t>::max();
	struct event
	{
		eid_t eid = nullevt;
		std::vector<eid_t> dependencies = {};
	};

	struct timeline_t : public std::unordered_set<eid_t>
	{
		timeline_t() = default;
		template<typename... Is>
		timeline_t(Is&&... is)
		{
			(this->insert(static_cast<eid_t>(static_cast<tz::hanval>(is))), ...);
		}
	};

	struct schedule
	{
		std::vector<event> events = {};
		timeline_t timeline = {};

		std::span<const eid_t> get_dependencies(eid_t evt) const;
		void dbgui();
	};

}

#endif // TZ_GL_API_SCHEDULE_HPP
