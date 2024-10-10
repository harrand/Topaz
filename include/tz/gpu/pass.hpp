#ifndef TOPAZ_GPU_PASS_HPP
#define TOPAZ_GPU_PASS_HPP
#include "tz/core/handle.hpp"
#include "tz/core/vector.hpp"
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
	};

	constexpr pass_flags operator|(pass_flags lhs, pass_flags rhs)
	{
		return static_cast<pass_flags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(pass_flags lhs, pass_flags& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	enum graphics_flag
	{
		dont_clear = 0x01, /// do not clear the colour targets of the pass - preserve their contents instead.
	};

	constexpr graphics_flag operator|(graphics_flag lhs, graphics_flag rhs)
	{
		return static_cast<graphics_flag>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(graphics_flag lhs, graphics_flag& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	struct pass_graphics_state
	{
		tz::v3f clear_colour = tz::v3f::zero();
		std::span<const resource_handle> colour_targets = {};
		resource_handle depth_target = tz::nullhand;
		graphics_flag flags = static_cast<graphics_flag>(0);
	};

	struct pass_compute_state
	{
		tz::v3u kernel = tz::v3u::zero();
	};

	struct pass_info
	{
		pass_graphics_state graphics = {};
		pass_compute_state compute = {};
		shader_handle shader = tz::nullhand;
		std::span<const resource_handle> resources = {};
		pass_flags flags = static_cast<pass_flags>(0);
		const char* name = "";
	};

	using pass_handle = tz::handle<pass_info>;
	std::expected<pass_handle, tz::error_code> create_pass(pass_info);

	void destroy_pass(pass_handle);
}

#endif // TOPAZ_GPU_PASS_HPP