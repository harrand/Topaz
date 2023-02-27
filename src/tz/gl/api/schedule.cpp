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

	void schedule::dbgui()
	{
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Render Graph");
		for(const event& evt : this->events)
		{
			std::string label = "Renderer ";
			label += std::to_string(evt.eid);
			if(evt.dependencies.size() && ImGui::TreeNode(label.c_str()))	
			{
				for(eid_t eid : evt.dependencies)
				{
					ImGui::Text("%u", eid);
				}
				ImGui::TreePop();
			}
		}
	}
}
