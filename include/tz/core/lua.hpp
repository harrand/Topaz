#ifndef TOPAZ_LUA_HPP
#define TOPAZ_LUA_HPP
#include "tz/topaz.hpp"
#include "tz/core/error.hpp"
#include <filesystem>
#include <expected>

namespace tz
{
	using lua_fn = int(*)();
	struct lua_nil{};
	/**
	 * @ingroup tz_core
	 * Attempt to execute a local lua file on the current thread.
	 * @param path Path to a local file containing lua code.
	 *
	 * @return @ref tz::error_code::precondition_failure If the provided path was invalid.
	 * @return @ref tz::error_code::unknown_error If the executed code caused an error.
	 */
	tz::error_code lua_execute_file(std::filesystem::path path);
	/**
	 * @ingroup tz_core
	 * Attempt to execute some lua code on the current thread.
	 * @param lua_src String containing lua code to execute.
	 *
	 * @return @ref tz::error_code::unknown_error If the executed code caused an error.
	 */
	tz::error_code lua_execute(std::string_view lua_src);

	tz::error_code lua_set_nil(std::string_view varname);
	tz::error_code lua_set_emptytable(std::string_view varname);
	tz::error_code lua_set_bool(std::string_view varname, bool v);
	tz::error_code lua_set_int(std::string_view varname, std::int64_t v);
	tz::error_code lua_set_number(std::string_view varname, double v);
	tz::error_code lua_set_string(std::string_view varname, std::string str);
	tz::error_code lua_define_function(std::string_view varname, lua_fn fn);

	std::expected<bool, tz::error_code> lua_get_bool(std::string_view varname);
	std::expected<std::int64_t, tz::error_code> lua_get_int(std::string_view varname);
	std::expected<double, tz::error_code> lua_get_number(std::string_view varname);
	std::expected<std::string, tz::error_code> lua_get_string(std::string_view varname);

	std::expected<bool, tz::error_code> lua_stack_get_bool(std::size_t id);
	std::expected<std::int64_t, tz::error_code> lua_stack_get_int(std::size_t id);
	std::expected<double, tz::error_code> lua_stack_get_number(std::size_t id);
	std::expected<std::string, tz::error_code> lua_stack_get_string(std::size_t id);

	std::string lua_debug_callstack();
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