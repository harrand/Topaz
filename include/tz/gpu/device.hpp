#ifndef TZ_GPU_DEVICE_HPP
#define TZ_GPU_DEVICE_HPP
#include <string>
#include <span>
#include <cstddef>
#include <array>

namespace tz::gpu
{
	enum class error_code
	{
		success,
		partial_success,
		precondition_failure,
		unknown_error,
		_count
	};
	constexpr std::array<const char*, static_cast<int>(error_code::_count)> error_code_names
	{
		"success",
		"partial success",
		"precondition failure",
		"unknown error"
	};

	enum class device_type
	{
		gpu,
		integrated_gpu,
		cpu
	};

	struct hardware
	{
		std::string name;
		std::uint64_t vram_size;
		device_type type;
	};

	error_code iterate_hardware(std::span<hardware> devices, std::size_t* hardware_count = nullptr);
}

#endif // TZ_GPU_DEVICE_HPP