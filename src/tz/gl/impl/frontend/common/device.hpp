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
			std::size_t id = 0;
			for(R& renderer : this->renderers)
			{
				std::string id_str = std::string("Renderer ") + std::to_string(id++);
				if(ImGui::CollapsingHeader(id_str.c_str()))
				{
					renderer.dbgui();
				}
			}
		}

		// Derived needs to define create_renderer still. They can use emplace_renderer as a helper function.
	protected:
		template<typename... Args>
		tz::gl::RendererHandle emplace_renderer(Args&&... args)
		{
			this->renderers.emplace_back(std::forward<Args>(args)...);
			return static_cast<tz::HandleValue>(this->renderers.size() - 1);
		}
	private:
		std::vector<R> renderers;
	};
}

#endif // TOPAZ_GL2_IMPL_FRONTEND_COMMON_DEVICE_HPP
