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

	enum pass_flag
	{
	};

	constexpr pass_flag operator|(pass_flag lhs, pass_flag rhs)
	{
		return static_cast<pass_flag>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(pass_flag lhs, pass_flag& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Specifies optional behaviours for a graphics pass.
	 **/
	enum graphics_flag
	{
		/// Do not clear the colour target images prior to rendering - whatever data they held previously will be loaded prior to rendering.
		dont_clear = 0b0001,
		/// Disable depth testing - i.e fragments that are behind another may still draw over it.
		no_depth_test = 0b0010,
		/// Disable depth writes - i.e when a depth test is passed by a fragment, the old depth value is not overwritten, but stays as it was before. @note Depth writes are always disabled if depth testing is disabled (see @ref graphics_flag::no_depth_test).
		no_depth_write = 0b0100,

	};

	constexpr graphics_flag operator|(graphics_flag lhs, graphics_flag rhs)
	{
		return static_cast<graphics_flag>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(graphics_flag lhs, graphics_flag& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Specifies face culling behaviour of a graphics pass.
	 **/
	enum class cull
	{
		/// Cull front and back faces.
		both,
		/// Cull the front face.
		front,
		/// Cull the back face.
		back,
		/// Do not perform any face culling.
		none
	};

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Specifies creation flags for a new graphics pass.
	 *
	 * See @ref tz::gpu::pass_info for usage.
	 **/
	struct pass_graphics_state
	{
		/// When a colour target is cleared, what colour (RGBA normalised floats) should it be cleared to?
		tz::v3f clear_colour = tz::v3f::zero();
		/// List of all colour targets. The n'th colour target specified here will be the n'th output of the fragment shader. If you want the window itself to be a colour target, pass @ref tz::gpu::window_resource.
		std::span<const resource_handle> colour_targets = {};
		/// Optional depth target. This will act as the depth image when performing depth testing/writes.
		resource_handle depth_target = tz::nullhand;
		/// Describe which faces will be culled during rendering.
		cull culling = cull::back;
		/// Specifies extra optional behaviour for the pass.
		graphics_flag flags = static_cast<graphics_flag>(0);
	};

	struct pass_compute_state
	{
		tz::v3u kernel = tz::v3u::zero();
	};

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Specifies creation flags for a new pass.
	 *
	 * See @ref tz::gpu::create_pass for usage.
	 **/
	struct pass_info
	{
		/// Describe the graphics state. If the pass is a compute pass, this is ignored.
		pass_graphics_state graphics = {};
		/// Describe the compute state. If the pass is a graphics pass, this is ignored.
		pass_compute_state compute = {};
		/// Corresponds to the shader that will run during execution. This must refer to a valid shader program created via @ref create_graphics_shader or @ref create_compute_shader.
		shader_handle shader = tz::nullhand;
		/// List of all resources used in the shader. The n'th resource specified here will be the resource available at binding 'n' within all stages of the provided shader program.
		std::span<const resource_handle> resources = {};
		/// Specifies extra optional behaviour for the pass.
		pass_flag flags = static_cast<pass_flag>(0);
		/// Name identifying the pass. Debug tools/error checking will refer to the pass by this name if you provide one. If you do not provide a name, one will be automatically generated for you.
		const char* name = "<untitled pass>";
	};

	using pass_handle = tz::handle<pass_info>;
	/**
	 * @ingroup tz_gpu_pass
	 * @brief Create a new pass.
	 * @return On success: A @ref pass_handle corresponding to the newly created pass.
	 * @return @ref tz::error_code::precondition_failure If you fail to provide a valid shader program.
	 * @return @ref tz::error_code::precondition_failure If you provide a malformed shader program. A shader program is only well-formed if it consists of only a compute shader, OR it consists of a single vertex and fragment shader.
	 * @return @ref tz::error_code::precondition_failure For a graphics pass if you fail to provide at least one colour target.
	 * @return @ref tz::error_code::precondition_failure For a graphics pass if you provide a colour target that is invalid. A valid colour target is either a.) the window resource (and you have opened a window), b.) an image resource created with @ref tz::gpu::image_flag::render_target
	 * @return @ref tz::error_code::precondition_failure For a graphics pass if any colour target provided does not exactly match the dimensions of all other provided colour targets. All colour targets must be images with the same dimensions. This does mean that if you provide the window resource as a colour target, all other colour targets must have the same dimensions as the window.
	 * @return @ref tz::error_code::oom If CPU memory is exhausted while trying to create the pass.
	 * @return @ref tz::error_code::voom If GPU memory is exhausted while trying to create the pass.
	 *
	 * Once you successfully create a pass, you can add it to a TODO: write docs on timelines
	 *
	 * Once you're done with a pass, you can manually destroy it using @ref destroy_pass.
	 * @note If you never destroy a pass manually, it will automatically be destroyed for you when you call @ref tz::terminate.
	 **/
	std::expected<pass_handle, tz::error_code> create_pass(pass_info);

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Manually destroy a pass.
	 *
	 * Passes can be quite heavy in the context of both CPU and GPU memory. This is due to internal components such as recorded command buffers, synchronisation primitives and compiled shader code.
	 *
	 * Because of this, it might sometimes be necessary to manually destroy a pass once you are certain you have no further use of it. This will free up a bunch of CPU/GPU memory.
	 *
	 * @warning If you fail to pass a valid @ref pass_handle to this function, the behaviour is undefined.
	 */
	void destroy_pass(pass_handle);
}

#endif // TOPAZ_GPU_PASS_HPP