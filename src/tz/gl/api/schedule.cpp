#include "tz/gl/api/schedule.hpp"
#include "tz/core/debug.hpp"
#include "tz/dbgui/dbgui.hpp"
#include <algorithm>

namespace tz::gl
{
	std::span<const eid_t> schedule::get_dependencies(eid_t evt) const
	{
		auto iter = std::find_if(this->events.begin(), this->events.end(), [evt](const auto& event){return event.eid == evt;});
		if(iter == this->events.end())
		{
			// schedule isn't even aware of this event. return empty.
			return {};
		}
		return iter->dependencies;
	}
}
