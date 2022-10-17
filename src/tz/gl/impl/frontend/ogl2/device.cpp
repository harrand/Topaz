#if TZ_OGL
#include "tz/core/profiling/zone.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/impl/frontend/ogl2/renderer.hpp"
#include "tz/gl/impl/frontend/ogl2/device.hpp"

namespace tz::gl
{

	DeviceOGL::DeviceOGL()
	{
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	}

	tz::gl::RendererHandle DeviceOGL::create_renderer(const RendererInfoOGL& info)
	{
		TZ_PROFZONE("OpenGL Frontend - Renderer Create (via Device)", TZ_PROFCOL_RED);
		this->renderers.push_back({info});
		return static_cast<tz::HandleValue>(this->renderers.size() - 1);
	}

	const RendererOGL& DeviceOGL::get_renderer(tz::gl::RendererHandle handle) const
	{
		return this->renderers[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))];
	}

	RendererOGL& DeviceOGL::get_renderer(tz::gl::RendererHandle handle)
	{
		return this->renderers[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))];
	}
	
	ImageFormat DeviceOGL::get_window_format() const
	{
		return ImageFormat::RGBA32;
	}

	void DeviceOGL::dbgui()
	{
		std::size_t id = 0;
		for(RendererOGL& renderer : this->renderers)
		{
			std::string id_str = std::string("Renderer ") + std::to_string(id++);
			if(ImGui::CollapsingHeader(id_str.c_str()))
			{
				renderer.dbgui();
			}
		}
	}
}

#endif // TZ_OGL
