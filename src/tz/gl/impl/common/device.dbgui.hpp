#include "lstate.h"
#include "tz/gl/api/device.hpp"
#include "imgui.h"
#undef assert

namespace tz::gl
{
	void common_device_dbgui(device_type<tz::gl::renderer_info> auto& device);
}