#ifndef TZ_LUA_API_HPP
#define TZ_LUA_API_HPP
#include "tz/lua/state.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/algorithms/static.hpp"

/// begin implementation of a new free function
#define LUA_BEGIN(name) int luafn_##name(void* s){tz::lua::state state{s};
// end implementation of a new free function.
#define LUA_END }
// register a previously-defined lua function for all thread-local lua states.
#define LUA_REGISTER_ALL(name) tz::lua::for_all_states([](tz::lua::state& s){s.assign_func(#name, luafn_##name);});
// register a previously-defined lua function for the current lua-state (the one on this thread)
#define LUA_REGISTER_ONE(name, state) s.assign_func(#name, luafn_##name)
// retrieve the name of the lua-struct harness for a given name. you probably don't care about this.
#define LUA_FN_NAME(name) luafn_##name

// namespaces (i.e libs)
// begin a new namespace
#define LUA_NAMESPACE_BEGIN(name) struct luans_##name { static constexpr tz::lua::impl::lua_register registers[] = {
// begin implementation of a new namespace function
#define LUA_NAMESPACE_FUNC_BEGIN(name) {.namestr = #name, .fnptr = [](void* s)->int{tz::lua::state state{s};
// end implementation of a new namespace function
#define LUA_NAMESPACE_FUNC_END }},
// end the current namespace.
#define LUA_NAMESPACE_END };};
// retrive the name of the lua-struct harness for a given namespace. you probably dont care about this.
#define LUA_NAMESPACE_NAME(name) luans_##name
// register a previously-defined lua namespace for all thread-local lua states.
#define LUA_NAMESPACE_REGISTER_ALL(name) tz::lua::for_all_states([](tz::lua::state& s){s.open_lib(#name, tz::lua::impl::lua_registers{luans_##name::registers});});

// classes (i.e types, object oriented programming)
// lua classes essentially wrap around existing classes. the existing class should have methods that follow the signature `int(tz::lua::state&)`.
// defining a lua class around that existing class allows you to easily expose those methods to be called in lua code.
// begin implementaton of a new class.
#define LUA_CLASS_BEGIN(name) struct luat_##name {
// begin a list of method declarations. you must only call LUA_METHOD from now on, until you call LUA_CLASS_METHODS_END
#define LUA_CLASS_METHODS_BEGIN static constexpr tz::lua::impl::lua_register registers[] = {
// expose a previously-defined method to lua. specifically, classname::methodname, which must have the signature (int(tz::lua::state&)).
// note that the equvalent of `this` is available through `lua_this`
#define LUA_METHOD(classname, methodname) {.namestr = #methodname, .fnptr = [](void* s)->int{tz::lua::state state{s}; auto& lua_this = state.stack_get_userdata<classname>(1); \
 return lua_this.methodname(state);\
  \
}},
// end the list of method declarations. you can only have one list per class.
#define LUA_CLASS_METHODS_END };
// end the implementation of the class.
#define LUA_CLASS_END };
// retrive the name of the lua-struct harness for a given class. you probably dont care about this.
#define LUA_CLASS_NAME(name) luat_##name
// register a previously-defined lua class for all thread-local lua states.
#define LUA_CLASS_REGISTER_ALL(name) tz::lua::for_all_states([](tz::lua::state& s){s.new_type(#name, tz::lua::impl::lua_registers{luat_##name::registers});});

// helper macro. push a lua-class by value onto the top of the lua stack, ensuring that the metatable madness is all sorted out under-the-hood.
// i know this is pretty ugly, but trust me, its less ugly than doing the metatable bollocks.
#define LUA_CLASS_PUSH(state, class_name, value) state.stack_push_userdata<class_name>(value); state.attach_to_top_userdata(#class_name, LUA_CLASS_NAME(class_name)::registers);

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
			else if constexpr(std::is_same_v<T, tz::lua::nil>)
			{
				// do nothing! its whatever
			}
			else
			{
				static_assert(std::is_void_v<T>, "Unrecognised lua argument type. Is it a supported type?");
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