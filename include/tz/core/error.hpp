#ifndef TOPAZ_CORE_ERROR_HPP
#define TOPAZ_CORE_ERROR_HPP
#include <string_view>
#include <array>

namespace tz
{
	/**
	 * @ingroup tz_core
	 * @brief Error codes for Topaz.
	 **/
	enum class error_code
	{
		/// Correct behaviour has occurred. No need to do any sanity checking.
		success,
		/// Nothing erroneous happened, but the process did not complete fully or otherwise provide an ideal result.
		partial_success,
		/// An error has occurred because some previously-required setup has not been complete. The most common cause of this is not initialising the engine via @ref tz::initialise
		precondition_failure,
		/// An error has occurred because the hardware currently being used is not suitable for the given task.
		hardware_unsuitable,
		/// An error has occurred due to an engine-side logic error, and you should submit a bug report.
		engine_bug,
		/// An error has occurred due to a serious hazard relating to the driver/hardware. This most likely means a graphics driver crash/hardware-lost.
		driver_hazard,
		/// An error has occurred, but it's not clear why.
		unknown_error,
		/// An error has occurred due to lack of CPU memory.
		oom,
		/// An error has occurred due to lack of GPU memory.
		voom,
		_count
	};

	namespace detail
	{
		constexpr std::array<const char*, static_cast<int>(error_code::_count)> error_code_strings
		{
			"success",
			"partial success",
			"precondition failure error",
			"hardware unsuitability error",
			"engine bug error",
			"driver hazard error",
			"unknown error",
			"out of CPU memory error",
			"out of GPU memory error",
		};
	}

	constexpr const char* error_code_name(error_code ec)
	{
		return detail::error_code_strings[static_cast<int>(ec)];
	}

	std::string_view last_error();
	void set_last_error(std::string errmsg);

}

#endif // TOPAZ_CORE_ERROR_HPP