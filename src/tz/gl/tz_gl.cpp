#include "tz/gl/tz_gl.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/core/imported_text.hpp"
#include ImportedTextHeader(tz_gl, lua)

namespace tz::gl
{
	// LUA API
	// new

	struct impl_tz_gl_renderer
	{
		impl_tz_gl_renderer(tz::gl::renderer_handle rh):
		rh(rh){}

		tz::gl::renderer& ren()
		{
			return tz::gl::get_device().get_renderer(this->rh);
		}

		int resource_count(tz::lua::state& state)
		{
			state.stack_push_uint(this->ren().resource_count());
			return 1;
		}

		int debug_get_name(tz::lua::state& state)
		{
			state.stack_push_string(this->ren().debug_get_name());
			return 1;
		}

		tz::gl::renderer_handle rh;
	};

	LUA_CLASS_BEGIN(impl_tz_gl_renderer)
		LUA_CLASS_METHODS_BEGIN
			LUA_METHOD(impl_tz_gl_renderer, resource_count)
			LUA_METHOD(impl_tz_gl_renderer, debug_get_name)
		LUA_CLASS_METHODS_END
	LUA_CLASS_END
	struct impl_tz_gl_device
	{
		int renderer_count(tz::lua::state& state)
		{
			state.stack_push_uint(tz::gl::get_device().renderer_count());
			return 1;
		}

		int get_renderer(tz::lua::state& state)
		{
			auto [_, rid] = tz::lua::parse_args<tz::lua::nil, unsigned int>(state);
			LUA_CLASS_PUSH(state, impl_tz_gl_renderer, impl_tz_gl_renderer{static_cast<tz::gl::renderer_handle>(static_cast<tz::hanval>(rid))});
			return 1;
		}
	};

	LUA_CLASS_BEGIN(impl_tz_gl_device)
		LUA_CLASS_METHODS_BEGIN
			LUA_METHOD(impl_tz_gl_device, renderer_count)
			LUA_METHOD(impl_tz_gl_device, get_renderer)
		LUA_CLASS_METHODS_END
	LUA_CLASS_END

	LUA_NAMESPACE_BEGIN(impl_tz_gl)
		LUA_NAMESPACE_FUNC_BEGIN(get_device)
			LUA_CLASS_PUSH(state, impl_tz_gl_device, {});
			return 1;
		LUA_NAMESPACE_FUNC_END
	LUA_NAMESPACE_END

	void lua_initialise(tz::lua::state& state)
	{
		state.assign_emptytable("tz.gl");
		state.new_type("impl_tz_gl_renderer", LUA_CLASS_NAME(impl_tz_gl_renderer)::registers);
		state.new_type("impl_tz_gl_device", LUA_CLASS_NAME(impl_tz_gl_device)::registers);

		// LUA_NAMESPACE_REGISTER_ONE basically
		state.open_lib("impl_tz_gl", LUA_NAMESPACE_NAME(impl_tz_gl)::registers);
		state.execute("tz.gl = impl_tz_gl");

		std::string tz_gl_lua_api{ImportedTextData(tz_gl, lua)};
		state.execute(tz_gl_lua_api.data());
	}
}