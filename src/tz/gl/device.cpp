#include "tz/gl/device.hpp"
#include "tz/core/profile.hpp"
#include "tz/lua/api.hpp"
#include <memory>

namespace tz::gl
{
	std::unique_ptr<device> dev = nullptr;

	device& get_device()
	{
		if(dev == nullptr)
		{
			TZ_PROFZONE("get_device() - first-time creation", 0xFFAAAA00);
			dev = std::make_unique<device>();	
		}
		return *dev;
	}

	void destroy_device()
	{
		if(dev != nullptr)
		{
			TZ_PROFZONE("tz::gl - destroy device", 0xFFAAAA00);
			// Why the hell this magic instead of assigning to nullptr, you ask?
			// It's very possible ~device() ends up invoking tz::gl::get_device(), which will be null during this dtor, so it tries to reconstruct it again and cause real problems. So the steps to fix this are:
			// Invoke dtor, tz::gl::get_device() remains valid throughout dtor usage this time.
			(*dev).~device();
			// Release the raw ptr from the unique_ptr. `dev` is now nullptr.
			auto* p = dev.release();
			// Free the memory without calling the dtor a second time.
			operator delete(p);
		}
	}

	// LUA API

	LUA_BEGIN(device_renderer_count)
		state.stack_push_uint(tz::gl::get_device().renderer_count());
		return 1;
	LUA_END

	LUA_BEGIN(device_get_renderer)
		std::size_t rid = state.stack_get_uint(1);	
		auto& ren = tz::gl::get_device().get_renderer(static_cast<tz::hanval>(rid));
		state.stack_push_ref(ren);
		return 1;
	LUA_END

	LUA_BEGIN(device_full_wait)
		tz::gl::get_device().full_wait();
		return 0;
	LUA_END

	LUA_BEGIN(device_frame_wait)
		tz::gl::get_device().frame_wait();
		return 0;
	LUA_END

	LUA_NAMESPACE_BEGIN(impl_tz_gl_device)
		LUA_NAMESPACE_FUNC_BEGIN(renderer_count)
			return 1;
		LUA_NAMESPACE_FUNC_END
	LUA_NAMESPACE_END

	void lua_initialise_device(tz::lua::state& state)
	{
		state.assign_emptytable("tz.gl._impl_device");
		state.assign_func("tz.gl._impl_device.renderer_count", LUA_FN_NAME(device_renderer_count));
		state.assign_func("tz.gl._impl_device.get_renderer", LUA_FN_NAME(device_get_renderer));
		state.assign_func("tz.gl._impl_device.full_wait", LUA_FN_NAME(device_full_wait));
		state.assign_func("tz.gl._impl_device.frame_wait", LUA_FN_NAME(device_frame_wait));

		state.execute("tz.gl.get_device = function() return tz.gl._impl_device end");
	}
}
