#ifndef TOPAZ_LUA_HPP
#define TOPAZ_LUA_HPP
#include "tz/topaz.hpp"
#include "tz/core/error.hpp"
#include <filesystem>
#include <expected>

namespace tz
{
	/**
	 * @ingroup tz_core
	 * @brief Represents the signature for a function that can be called from lua.
	 *
	 * You might be expecting `int(lua_State*)` if you're used to using Lua directly. However, lua headers are not available to you, so this is the only signature you will worry about.
	 * Like in default lua, the return value of any lua function represents how many values you have returned (put onto the stack).
	 * To retrieve an argument in a lua function, you can either:
	 * 		- Call @ref lua_stack_get_bool or similar. This is useful if you conditionally need only a particular argument.
	 *		- Call @ref lua_parse_args to retrieve all the arguments of your function at once. This is recommended for most use-cases.
	 *
	 * Example: C function callable in lua:
	 * ``` 
	 * int fnimpl()
	 * {
	 * 		auto [arg1, arg2] = tz::lua_parse_args<int, std::string>();
	 *		lua_push_int(arg1 * 2);
	 *		lua_push_string(arg2 + " is cool");
	 *		return 2;
	 * }
	 * // In your application initialisation code:
	 * tz::lua_define_function("my_cool_function", fnimpl);
	 * // In your lua code:
	 * ret1, ret2 = my_cool_function(123, "Bob" .. " Marley")
	 * ``` 
	 */
	using lua_fn = int(*)();
	struct lua_nil{};
	/**
	 * @ingroup tz_core
	 * @brief Attempt to execute a local lua file on the current thread.
	 * @param path Path to a local file containing lua code.
	 *
	 * @return @ref tz::error_code::precondition_failure If the provided path was invalid.
	 * @return @ref tz::error_code::unknown_error If the executed code caused an error.
	 */
	tz::error_code lua_execute_file(std::filesystem::path path);
	/**
	 * @ingroup tz_core
	 * @brief Attempt to execute some lua code on the current thread.
	 * @param lua_src String containing lua code to execute.
	 *
	 * @return @ref tz::error_code::unknown_error If the executed code caused an error.
	 */
	tz::error_code lua_execute(std::string_view lua_src);
	/**
	 * @ingroup tz_core
	 * @brief Set a variable in lua to be nil.
	 * @param varname Name of the variable to set.
	 * @return @ref tz::error_code::unknown_error If an error occurred.
	 */
	tz::error_code lua_set_nil(std::string_view varname);
	/**
	 * @ingroup tz_core
	 * @brief Set a variable in lua to be the empty table "{}"
	 * @param varname Name of the variable to set.
	 * @return @ref tz::error_code::unknown_error If an error occurred.
	 */
	tz::error_code lua_set_emptytable(std::string_view varname);
	/**
	 * @ingroup tz_core
	 * @brief Set a variable in lua to a new bool value.
	 * @param varname Name of the variable to set.
	 * @param v Value to set
	 * @return @ref tz::error_code::unknown_error If an error occurred.
	 */
	tz::error_code lua_set_bool(std::string_view varname, bool v);
	/**
	 * @ingroup tz_core
	 * @brief Set a variable in lua to a new int value.
	 * @param varname Name of the variable to set.
	 * @param v Value to set
	 * @return @ref tz::error_code::unknown_error If an error occurred.
	 */
	tz::error_code lua_set_int(std::string_view varname, std::int64_t v);
	/**
	 * @ingroup tz_core
	 * @brief Set a variable in lua to a new number value.
	 * @param varname Name of the variable to set.
	 * @param v Value to set
	 * @return @ref tz::error_code::unknown_error If an error occurred.
	 */
	tz::error_code lua_set_number(std::string_view varname, double v);
	/**
	 * @ingroup tz_core
	 * @brief Set a variable in lua to a new string value.
	 * @param varname Name of the variable to set.
	 * @param v Value to set
	 * @return @ref tz::error_code::unknown_error If an error occurred.
	 */
	tz::error_code lua_set_string(std::string_view varname, std::string str);
	/**
	 * @ingroup tz_core
	 * @brief Define a new function in lua.
	 * @param varname Name of the function when called in lua code.
	 * @param v Pointer to an existing function to expose to lua.
	 * @return @ref tz::error_code::unknown_error If an error occurred.
	 */
	tz::error_code lua_define_function(std::string_view varname, lua_fn fn);

	/**
	 * @ingroup tz_core
	 * @brief Retrieve the value of a bool variable.
	 * @param varname Name of the variable to retrieve.
	 * @return @ref tz::error_code::precondition_failure If such a variable does not exist, or does not match the type you requested.
	 */
	std::expected<bool, tz::error_code> lua_get_bool(std::string_view varname);
	/**
	 * @ingroup tz_core
	 * @brief Retrieve the value of a int variable.
	 * @param varname Name of the variable to retrieve.
	 * @return @ref tz::error_code::precondition_failure If such a variable does not exist, or does not match the type you requested.
	 */
	std::expected<std::int64_t, tz::error_code> lua_get_int(std::string_view varname);
	/**
	 * @ingroup tz_core
	 * @brief Retrieve the value of a number variable.
	 * @param varname Name of the variable to retrieve.
	 * @return @ref tz::error_code::precondition_failure If such a variable does not exist, or does not match the type you requested.
	 */
	std::expected<double, tz::error_code> lua_get_number(std::string_view varname);
	/**
	 * @ingroup tz_core
	 * @brief Retrieve the value of a string variable.
	 * @param varname Name of the variable to retrieve.
	 * @return @ref tz::error_code::precondition_failure If such a variable does not exist, or does not match the type you requested.
	 */
	std::expected<std::string, tz::error_code> lua_get_string(std::string_view varname);

	/**
	 * @ingroup tz_core
	 * @brief Retrieve a bool from the stack at the given index.
	 * @param id Position on the stack to retrieve.
	 * @return @ref tz::error_precondition_failure If the stack is too small, or the value at the position you specifies does not match the type you asked for.
	 */
	std::expected<bool, tz::error_code> lua_stack_get_bool(std::size_t id);
	/**
	 * @ingroup tz_core
	 * @brief Retrieve a int from the stack at the given index.
	 * @param id Position on the stack to retrieve.
	 * @return @ref tz::error_precondition_failure If the stack is too small, or the value at the position you specifies does not match the type you asked for.
	 */
	std::expected<std::int64_t, tz::error_code> lua_stack_get_int(std::size_t id);
	/**
	 * @ingroup tz_core
	 * @brief Retrieve a number from the stack at the given index.
	 * @param id Position on the stack to retrieve.
	 * @return @ref tz::error_precondition_failure If the stack is too small, or the value at the position you specifies does not match the type you asked for.
	 */
	std::expected<double, tz::error_code> lua_stack_get_number(std::size_t id);
	/**
	 * @ingroup tz_core
	 * @brief Retrieve a string from the stack at the given index.
	 * @param id Position on the stack to retrieve.
	 * @return @ref tz::error_precondition_failure If the stack is too small, or the value at the position you specifies does not match the type you asked for.
	 */
	std::expected<std::string, tz::error_code> lua_stack_get_string(std::size_t id);

	/**
	 * @ingroup tz_core
	 * @brief Retreieve a string describing the lua callstack right now.
	 *
	 * This might be useful for debugging purposes.
	 */
	std::string lua_debug_callstack();
	/**
	 * @ingroup tz_core
	 * @brief Retreieve a string describing the entire lua stack right now.
	 *
	 * This might be useful for debugging purposes.
	 */
	std::string lua_debug_stack();

	template<int F, int L>
	struct static_for_t
	{
		template<typename Functor>
		static inline constexpr void apply(const Functor& f)
		{
			if(F < L)
			{
				f(std::integral_constant<int, F>{});
				static_for_t<F + 1, L>::apply(f);
			}
		}

		template<typename Functor>
		inline constexpr void operator()(const Functor& f) const
		{
			apply(f);
		}
	};

	template<int N>
	struct static_for_t<N, N>
	{
		template<typename Functor>
		static inline constexpr void apply([[maybe_unused]] const Functor& f){}
	};

	template<int F, int L>
	inline constexpr static_for_t<F, L> static_for = {};

	/**
	 * @ingroup tz_core
	 * @brief Retreve a set of arguments from the stack.
	 * @pre The values on the top of the stack must perfectly correspond to the type parameters you pass in, otherwise a runtime error will occur.
	 * You are recommended to use this in your own @ref lua_fn functions that you expose to lua via @ref lua_define_function.
	 */
	template<typename... Ts>
	std::tuple<Ts...> lua_parse_args()
	{
		std::tuple<Ts...> ret;
		static_for<0, sizeof...(Ts)>([&ret]([[maybe_unused]] auto i) constexpr
		{
			using T = std::decay_t<decltype(std::get<i.value>(std::declval<std::tuple<Ts...>>()))>;	
			auto& v = std::get<i.value>(ret);
			if constexpr(std::is_same_v<T, bool>)
			{
				v = tz_must(lua_stack_get_bool(i.value + 1));
			}
			else if constexpr(std::is_same_v<T, float>)
			{
				v = tz_must(lua_stack_get_number(i.value + 1));
			}
			else if constexpr(std::is_same_v<T, std::int64_t> || std::is_same_v<T, int>)
			{
				v = tz_must(lua_stack_get_int(i.value + 1));
			}
			else if constexpr(std::is_same_v<T, std::string>)
			{
				v = tz_must(lua_stack_get_string(i.value + 1));
			}
			else if constexpr(std::is_same_v<T, lua_nil>)
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

#endif // TOPAZ_LUA_HPP