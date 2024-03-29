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
}
