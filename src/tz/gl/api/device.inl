#include "tz/core/profile.hpp"
#include "imgui.h"
#undef assert
namespace tz::gl
{
	template<renderer_type R>
	const R& device_common<R>::get_renderer(tz::gl::renderer_handle handle) const
	{
		return this->renderers[static_cast<std::size_t>(static_cast<tz::hanval>(handle))];
	}

	template<renderer_type R>
	R& device_common<R>::get_renderer(tz::gl::renderer_handle handle)
	{
		return this->renderers[static_cast<std::size_t>(static_cast<tz::hanval>(handle))];
	}

	template<renderer_type R>
	void device_common<R>::destroy_renderer(tz::gl::renderer_handle handle)
	{
		std::size_t h = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
#if TZ_DEBUG
		bool free_list_contains = std::find(this->free_list.begin(), this->free_list.end(), h) != this->free_list.end();
		tz::assert(!free_list_contains, "Detected double-destroy of renderer of handle value %zu", h);
		tz::assert(this->renderers.size() > h, "Detected attempted destroy of renderer of invalid handle value %zu. device renderer storage does not have the capacity for this, meaning no renderer with this handle was ever returned by this device.", h);

#endif // TZ_DEBUG
		R tmp = R::null();
		std::swap(tmp, this->renderers[h]);
		this->free_list.push_back(h);
	}

	template<renderer_type R>
	std::size_t device_common<R>::renderer_count() const
	{
		return this->renderers.size() - this->free_list.size();
	}

	template<renderer_type R>
	const tz::gl::schedule& device_common<R>::render_graph() const
	{
		return this->render_schedule;
	}

	template<renderer_type R>
	tz::gl::schedule& device_common<R>::render_graph()
	{
		return this->render_schedule;
	}

	template<renderer_type R>
	void device_common<R>::render()
	{
		TZ_PROFZONE("device - render", 0xFFAAAA00);
		for(eid_t evt : this->render_schedule.timeline)
		{
			this->renderers[evt].render();
		}
	}

	template<renderer_type R>
	tz::gl::renderer_handle device_common<R>::emplace_renderer(const tz::gl::renderer_info& rinfo)
	{
		// If free list is empty, we need to expand our storage and retrieve a new handle.
		std::size_t rid;
		if(this->free_list.empty())
		{
			this->renderers.emplace_back(rinfo);
			rid = this->renderers.size() - 1;
		}
		else
		{
			// We destroyed a renderer in the past and now the free list contains a reference to the null renderer at its place. We can re-use this position.
			// If we destroyed a renderer in the past, let's re-use its handle. The renderer at the position will be a null renderer.
			rid = this->free_list.back();
			this->free_list.pop_back();
			this->renderers[rid] = R{rinfo};
		}
		this->post_add_renderer(rid, rinfo);
		return static_cast<hanval>(rid);
	}

	template<renderer_type R>
	void device_common<R>::internal_clear()
	{
		this->renderers.clear();
	}

	template<renderer_type R>
	void device_common<R>::post_add_renderer(std::size_t rid, const tz::gl::renderer_info& rinfo)
	{
		const auto& evts = this->render_graph().events;
		auto iter = std::find_if(evts.begin(), evts.end(), [&rid](const auto& evt){return evt.eid == rid;});
		if(iter == this->render_graph().events.end())
		{
			tz::gl::event& evt = this->render_graph().events.emplace_back();
			evt.eid = rid;
			auto deps = rinfo.get_dependencies();
			for(const auto& dep : deps)
			{
				evt.dependencies.push_back(static_cast<eid_t>(static_cast<std::size_t>(static_cast<tz::hanval>(dep))));
			}
		}
	}

	template<tz::gl::device_type<tz::gl::renderer_info> T>
	void common_device_dbgui(T& device)
	{
		const std::size_t renderer_count = device.renderer_count();

		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "tz::gl::get_device()");
		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Summary");
		ImGui::Text("- The device currently stores %zu renderers", renderer_count);
		ImGui::Text("- The image_format of the window is %s", detail::image_format_strings[static_cast<int>(device.get_window_format())]);
		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Renderers");
		static bool display_internal_renderers = false;
		ImGui::Checkbox("Display Internal Renderers", &display_internal_renderers);
		static int id = 0;
		if(renderer_count == 0)
		{
			return;
		}
		ImGui::SliderInt("Renderer ID", &id, 0, renderer_count - 1);
		ImGui::Indent();
		const auto& renderer = device.get_renderer(static_cast<tz::hanval>(id));
		if(renderer.get_options().contains(tz::gl::renderer_option::_internal) && !display_internal_renderers)
		{
			ImGui::Text("Internal Renderer");
			ImGui::Spacing();
			if(id > 0)
			{
				if(ImGui::Button("<<")){while(device.get_renderer(static_cast<tz::hanval>(id)).get_options().contains(tz::gl::renderer_option::_internal) && id > 0){id--;}}
				ImGui::SameLine();
				if(ImGui::Button("Prev")){id--;}
			}
			if(std::cmp_less(id, (renderer_count - 1)))
			{
				ImGui::SameLine();
				if(ImGui::Button("Next")){id++;}
				ImGui::SameLine();
				if(ImGui::Button(">>")){while(device.get_renderer(static_cast<tz::hanval>(id)).get_options().contains(tz::gl::renderer_option::_internal) && std::cmp_less(id, (renderer_count - 1))){id++;}}
			}
		}
		else
		{
			device.get_renderer(static_cast<tz::hanval>(id)).dbgui();
		}
		ImGui::Unindent();
		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Render Graph");
		if(ImGui::BeginTabBar("#rendergraph"))
		{
			const auto& sched = device.render_graph();
			constexpr ImVec4 edge_default{1.0f, 1.0f, 1.0f, 1.0f};
			constexpr ImVec4 edge_dependency{0.6f, 0.0f, 0.0f, 1.0f};
			constexpr ImVec4 edge_renderwait{0.4f, 0.5f, 1.0f, 1.0f};
			constexpr ImVec4 edge_implicit{0.6f, 0.6f, 0.0f, 1.0f};
			if(ImGui::BeginTabItem("Timeline"))
			{
				static bool hide_internal = true;
				static bool show_chronological_ranks = false;
				ImGui::Checkbox("Hide Internal Renderers", &hide_internal);
				ImGui::Checkbox("Show Chronological Ranks", &show_chronological_ranks);
				auto max_rank = sched.max_chronological_rank();
				for(std::size_t r = 0; r <= max_rank; r++)
				{
					for(std::size_t j = 0; j < sched.timeline.size(); j++)
					{
						auto& ren = device.get_renderer(static_cast<tz::hanval>(sched.timeline[j]));
						if(sched.chronological_rank_eid(sched.timeline[j]) == r)
						{
							std::string namedata{ren.debug_get_name()};
							if(show_chronological_ranks)
							{
								namedata += " (" + std::to_string(r) + ")";
							}
							if(ImGui::Button(namedata.c_str()))
							{
								id = sched.timeline[j];
							}
							ImGui::SameLine();
						}
					}
					ImGui::Spacing();
				}
				#if TZ_DEBUG
				if(!hide_internal)
				{			
					for(std::size_t i = 0; i < 2; i++)
					{
						if(device.renderer_count() > i && ImGui::Button(device.get_renderer(static_cast<tz::hanval>(i)).debug_get_name().data()))
						{
							id = i;
							display_internal_renderers = true;
						}
						if(i < 1)
						{
							ImGui::SameLine();
							ImGui::TextColored(edge_implicit, " -> ");
							ImGui::SameLine();
						}
					}
				}
				#endif
				ImGui::Spacing();
				ImGui::Indent();
				ImGui::Text("("); ImGui::SameLine(); ImGui::TextColored(edge_default, "->"); ImGui::SameLine(); ImGui::Text("Default)"); ImGui::SameLine();
				ImGui::Text("("); ImGui::SameLine(); ImGui::TextColored(edge_dependency, "->"); ImGui::SameLine(); ImGui::Text("Dependency)"); ImGui::SameLine();
				ImGui::Text("("); ImGui::SameLine(); ImGui::TextColored(edge_renderwait, "->"); ImGui::SameLine(); ImGui::Text("Render Wait)"); ImGui::SameLine();
				if(!hide_internal)
				{
					ImGui::Text("("); ImGui::SameLine(); ImGui::TextColored(edge_implicit, "->"); ImGui::SameLine(); ImGui::Text("Internal)");
				}
				ImGui::Unindent();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
}
