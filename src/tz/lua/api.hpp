#ifndef TZ_LUA_API_HPP
#define TZ_LUA_API_HPP
#include "tz/lua/state.hpp"
#include "tz/core/debug.hpp"

// `s` == underlying lua_State*. `state` == tz::lua::state
// free functions:
#define LUA_BEGIN(name) int luafn_##name(void* s){tz::lua::state state{s};

#define LUA_END }
#define LUA_REGISTER_ALL(name) tz::lua::for_all_states([](tz::lua::state& s){s.assign_func(#name, luafn_##name);});
#define LUA_REGISTER_ONE(name, state) s.assign_func(#name, luafn_##name)
#define LUA_FN_NAME(name) luafn_##name

// namespaces (i.e libs)
#define LUA_NAMESPACE_BEGIN(name) struct luans_##name { static constexpr tz::lua::impl::lua_register registers[] = {
#define LUA_NAMESPACE_FUNC_BEGIN(name) {.namestr = #name, .fnptr = [](tz::lua::state& s)->int{
#define LUA_NAMESPACE_FUNC_END }},
#define LUA_NAMESPACE_END };};

#define LUA_NAMESPACE_REGISTER_ALL(name) tz::lua::for_all_states([](tz::lua::state& s){s.open_lib(#name, tz::lua::impl::lua_registers{luans_##name::registers, std::size(luans_##name::registers)});});

namespace tz::lua
{
	void api_initialise(state& s);
}

// example: define in a TU
//LUA_BEGIN(test_me_please)
//	tz::report("test successful!");
//	return 0;
//LUA_END

// usage: somewhere during runtime, invoke:
// LUA_REGISTER_ALL(test_me_please)
// which makes the function resident to all lua states on both the main thread and the job system worker threads.

#endif // TZ_LUA_API_HPP