#ifndef TZ_LUA_STATE_HPP
#define TZ_LUA_STATE_HPP
#include "tz/core/data/handle.hpp"
#include <string>
#include <cstdint>
#include <type_traits>
#include <functional>
#include <optional>
#include <thread>

namespace tz::lua
{
	/**
	* @ingroup tz_lua_cpp
	* Represents a lua state. To retrieve the main lua state, see @ref tz::lua::get_state()
	*/
	class state
	{
	public:
		state() = default;
		state(void* lstate);
		/**
		* Query as to whether the state is valid.
		* @return True if the state is valid, false otherwise.
		*/
		bool valid() const;
		/**
		* Attempt to execute a lua source file. The code is instantly executed, and returns on completion.
		* @param path The relative path locating the lua source file (with extension).
		* @param assert_on_failure Whether Topaz should assert on the code running without any errors.
		* @return Whether the executed code experienced any errors.
		*/
		bool execute_file(const char* path, bool assert_on_failure = true) const;
		/**
		* Attempt to execute a lua source string. The code is instantly executed, and returns on completion.
		* @param lua_src String containing lua source code, appropriate for the state.
		* @param assert_on_failure Whether Topaz should assert on the code running without any errors.
		* @return Whether the executed code experienced any errors.
		*/
		bool execute(const char* lua_src, bool assert_on_failure = true) const;

		bool assign_nil(const char* varname) const;
		bool assign_emptytable(const char* varname) const;
		bool assign_bool(const char* varname, bool b) const;
		bool assign_float(const char* varname, float f) const;
		bool assign_double(const char* varname, double d) const;
		bool assign_int(const char* varname, std::int64_t i) const;
		bool assign_uint(const char* varname, std::uint64_t u) const;
		bool assign_func(const char* varname, auto anon_ptr) const
		{
			using T = std::decay_t<decltype(anon_ptr)>;
			if constexpr(std::is_pointer_v<T>)
			{
				return this->assign_func(varname, reinterpret_cast<void*>(anon_ptr));
			}
			return false;
		}
		bool assign_func(const char* varname, void* func_ptr) const;
		bool assign_string(const char* varname, std::string str) const;

		std::optional<bool> get_bool(const char* varname) const;
		std::optional<float> get_float(const char* varname) const;
		std::optional<double> get_double(const char* varname) const;
		std::optional<std::int64_t> get_int(const char* varname) const;
		std::optional<std::uint64_t> get_uint(const char* varname) const;
		std::string collect_stack() const;
		const std::string& get_last_error() const;
		std::thread::id get_owner_thread_id() const;
	private:
		bool impl_check_stack(std::size_t sz) const;
		mutable std::string last_error = "";
		void* lstate = nullptr;
		std::thread::id owner = std::this_thread::get_id();
	};

	/**
	* @ingroup tz_lua_cpp
	* Retrieve the main lua @ref tz::lua::state.
	*/
	state& get_state();

	using state_applicator = std::function<void(state&)>;
	void for_all_states(state_applicator fn);
}

#endif // TZ_LUA_STATE_HPP
