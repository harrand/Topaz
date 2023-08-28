#ifndef TZ_LUA_API_HPP
#define TZ_LUA_API_HPP
#include "tz/lua/state.hpp"
#include "tz/core/debug.hpp"

#define LUA_BEGIN(name) int luafn_##name(void* state){

#define LUA_END }
#define LUA_REGISTER(name) tz::lua::for_all_states([](tz::lua::state& s){s.assign_func(#name, luafn_##name);});

// example: define in a TU
//LUA_BEGIN(test_me_please)
//	tz::report("test successful!");
//	return 0;
//LUA_END

// usage: somewhere during runtime, invoke:
// LUA_REGISTER(test_me_please)
// which makes the function resident to all lua states on both the main thread and the job system worker threads.

#endif // TZ_LUA_API_HPP