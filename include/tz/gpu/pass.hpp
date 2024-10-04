#ifndef TOPAZ_GPU_PASS_HPP
#define TOPAZ_GPU_PASS_HPP
#include "tz/core/handle.hpp"
#include "tz/gpu/resource.hpp"
#include "tz/gpu/shader.hpp"

namespace tz::gpu
{
	/**
	 * @ingroup tz_gpu
	 * @defgroup tz_gpu_pass Render/Compute Passes
	 * @brief Documentation for render/compute passes - each a single node within a render graph.
	 **/

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Describes what kind of GPU work a pass will involve. 
	 **/
	enum class pass_type
	{
		/// Render Pass - involves rendering some kind of geometry via a vertex and fragment shader.
		render,
		/// Compute Pass - involves bespoke GPU-side processing via a compute shader.
		compute
	};

	enum pass_flags
	{
		none = 0x00
	};

	constexpr pass_flags operator|(pass_flags lhs, pass_flags rhs)
	{
		return static_cast<pass_flags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(pass_flags lhs, pass_flags& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	struct pass_info
	{
		shader_handle shader = tz::nullhand;
		std::span<const resource_handle> resources = {};
		pass_flags flags = pass_flags::none;
		const char* debug_name = "";
	};

	using pass_handle = tz::handle<pass_info>;
	std::expected<pass_handle, tz::error_code> create_pass(pass_info);
}

#endif // TOPAZ_GPU_PASS_HPP