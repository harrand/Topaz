#ifndef TZ_LUA_API_HPP
#define TZ_LUA_API_HPP
#include "tz/lua/state.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/algorithms/static.hpp"

// `s` == underlying lua_State*. `state` == tz::lua::state
// free functions:
#define LUA_BEGIN(name) int luafn_##name(void* s){tz::lua::state state{s};

#define LUA_END }
#define LUA_REGISTER_ALL(name) tz::lua::for_all_states([](tz::lua::state& s){s.assign_func(#name, luafn_##name);});
#define LUA_REGISTER_ONE(name, state) s.assign_func(#name, luafn_##name)
#define LUA_FN_NAME(name) luafn_##name

// namespaces (i.e libs)
#define LUA_NAMESPACE_BEGIN(name) struct luans_##name { static constexpr tz::lua::impl::lua_register registers[] = {
#define LUA_NAMESPACE_FUNC_BEGIN(name) {.namestr = #name, .fnptr = [](void* s)->int{tz::lua::state state{s};
#define LUA_NAMESPACE_FUNC_END }},
#define LUA_NAMESPACE_END };};
#define LUA_NAMESPACE_NAME(name) luans_##name

#define LUA_NAMESPACE_REGISTER_ALL(name) tz::lua::for_all_states([](tz::lua::state& s){s.open_lib(#name, tz::lua::impl::lua_registers{luans_##name::registers});});

#define LUA_CLASS_BEGIN(name) struct luat_##name {
#define LUA_CLASS_METHODS_BEGIN static constexpr tz::lua::impl::lua_register registers[] = {
#define LUA_METHOD(classname, methodname) {.namestr = #methodname, .fnptr = [](void* s)->int{tz::lua::state state{s}; auto& lua_this = state.stack_get_userdata<classname>(1); \
 return lua_this.methodname(state);\
}},
#define LUA_CLASS_METHODS_END };
#define LUA_CLASS_END };
#define LUA_CLASS_NAME(name) luat_##name
#define LUA_CLASS_REGISTER_ALL(name) tz::lua::for_all_states([](tz::lua::state& s){s.new_type(#name, tz::lua::impl::lua_registers{luat_##name::registers});});

namespace tz::lua
{
	void api_initialise(state& s);
	template<typename... Ts>
	std::tuple<Ts...> parse_args(state& s)
	{
		std::tuple<Ts...> ret;
		tz::static_for<0, sizeof...(Ts)>([&ret, &s]([[maybe_unused]] auto i) constexpr
		{
			using T = std::decay_t<decltype(std::get<i.value>(std::declval<std::tuple<Ts...>>()))>;	
			auto& v = std::get<i.value>(ret);
			if constexpr(std::is_same_v<T, bool>)
			{
				v = s.stack_get_bool(i.value + 1);
			}
			else if constexpr(std::is_same_v<T, float>)
			{
				v = s.stack_get_float(i.value + 1);
			}
			else if constexpr(std::is_same_v<T, double>)
			{
				v = s.stack_get_double(i.value + 1);
			}
			else if constexpr(std::is_same_v<T, std::int64_t> || std::is_same_v<T, int>)
			{
				v = s.stack_get_int(i.value + 1);
			}
			else if constexpr(std::is_same_v<T, std::uint64_t> || std::is_same_v<T, unsigned int>)
			{
				v = s.stack_get_uint(i.value + 1);
			}
			else if constexpr(std::is_same_v<T, std::string>)
			{
				v = s.stack_get_string(i.value + 1);
			}
			else
			{
				static_assert(!std::is_void_v<T>, "Unrecognised lua argument type. Is it a supported type?");
			}
		});
		return ret;
	}
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