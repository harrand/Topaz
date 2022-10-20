#ifndef TOPAZ_GL2_IMPL_FRONTEND_COMMON_DEVICE_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_COMMON_DEVICE_HPP
#include "tz/gl/api/renderer.hpp"
#include "imgui.h"
#include <vector>
#include <string>

namespace tz::gl
{
	template<RendererType R>
	class DeviceCommon
	{
	public:
		DeviceCommon() = default;
		const R& get_renderer(tz::gl::RendererHandle handle) const
		{
			return this->renderers[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))];
		}

		R& get_renderer(tz::gl::RendererHandle handle)
		{
			return this->renderers[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))];
		}

		void dbgui()
		{
			static int id = 0;
			const std::size_t renderer_count = this->renderers.size();
			if(renderer_count == 0)
			{
				return;
			}
			ImGui::DragInt("Renderer ID", &id, 0.2f, 0, renderer_count - 1);
			ImGui::Indent();
			this->renderers[id].dbgui();
			ImGui::Unindent();
		}

		void destroy_renderer(tz::gl::RendererHandle handle)
		{
			std::size_t h = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
#if TZ_DEBUG
			bool free_list_contains = std::find(this->free_list.begin(), this->free_list.end(), h) != this->free_list.end();
			tz_assert(!free_list_contains, "Detected double-destroy of renderer of handle value %zu", h);
			tz_assert(this->renderers.size() > h, "Detected attempted destroy of renderer of invalid handle value %zu. Device renderer storage does not have the capacity for this, meaning no renderer with this handle was ever returned by this device.", h);

#endif // TZ_DEBUG
			this->renderers[h] = R::null();
			this->free_list.push_back(h);
		}

		std::size_t renderer_count() const
		{
			return this->renderers.size() - this->free_list.size();
		}

		// Derived needs to define create_renderer still. They can use emplace_renderer as a helper function.
	protected:
		template<typename... Args>
		tz::gl::RendererHandle emplace_renderer(Args&&... args)
		{
			// If free list is empty, we need to expand our storage and retrieve a new handle.
			if(this->free_list.empty())
			{
				this->renderers.emplace_back(std::forward<Args>(args)...);
				return static_cast<tz::HandleValue>(this->renderers.size() - 1);
			}
			else
			{
				// We destroyed a renderer in the past and now the free list contains a reference to the null renderer at its place. We can re-use this position.
				// If we destroyed a renderer in the past, let's re-use its handle. The renderer at the position will be a null renderer.
				tz::gl::RendererHandle h = static_cast<tz::HandleValue>(this->free_list.back());
				this->free_list.pop_back();
				this->get_renderer(h) = R{std::forward<Args>(args)...};
				return h;
			}
		}

		void internal_clear()
		{
			this->renderers.clear();
		}
	private:
		std::vector<R> renderers;
		std::vector<std::size_t> free_list = {};
	};
}

#endif // TOPAZ_GL2_IMPL_FRONTEND_COMMON_DEVICE_HPP
