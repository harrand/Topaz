#ifndef TZ_GL_API_SCHEDULE_HPP
#define TZ_GL_API_SCHEDULE_HPP
#include "tz/core/data/handle.hpp"
#include "tz/core/debug.hpp"
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

	struct timeline_t : public std::vector<eid_t>
	{
		timeline_t() = default;
		template<typename... Is>
		timeline_t(Is&&... is)
		{
			(this->push_back(static_cast<eid_t>(static_cast<tz::hanval>(is))), ...);
		}
	};

	struct schedule
	{
		std::vector<event> events = {};
		timeline_t timeline = {};

		std::span<const eid_t> get_dependencies(eid_t evt) const;

		template<typename T0, typename... T>
		void add_dependencies(T0 evth, T... deps)
		{
			auto evt = static_cast<eid_t>(static_cast<tz::hanval>(evth));
			auto iter = std::find_if(this->events.begin(), this->events.end(), [evt](const auto& event){return event.eid == evt;});
			tz::assert(iter != this->events.end());
			(iter->dependencies.push_back(static_cast<eid_t>(static_cast<tz::hanval>(deps))), ...);
		}
		void dbgui();
	};

}

#endif // TZ_GL_API_SCHEDULE_HPP
