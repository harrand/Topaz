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
	 * @defgroup tz_gpu_pass Passes
	 * @brief Documentation for render/compute passes - each a single node within @ref tz_gpu_graph.
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
		tz::v4f clear_colour = {0.0f, 0.0f, 0.0f, 1.0f};
		/// Scissor rectangle, components represent `{offsetx, offsety, extentx, extenty}` in pixels. If all values are -1, the scissor offset will be `{0, 0}` and the extent equal to the dimensions of the @ref colour_targets you have provided.
		tz::v4u scissor = tz::v4u::filled(-1);
		/// List of all colour targets. The n'th colour target specified here will be the n'th output of the fragment shader. If you want the window itself to be a colour target, pass @ref tz::gpu::window_resource. You must provide at least one colour target, and all colour targets must have the same dimensions.
		std::span<const resource_handle> colour_targets = {};
		/// Optional depth target. This will act as the depth image when performing depth testing/writes.
		resource_handle depth_target = tz::nullhand;
		/**
		 * @brief Buffer containing indices drawn in a single invocation of the pass.
		 *
		 * The memory contents of the index buffer are expected to be:
		 *
		 * `{indices...}`
		 *
		 * Where a single index is equal to a @ref index_t.
		 * - The number of indices in the buffer does not affect how many vertices ultimately get drawn. That is always controlled by the @ref draw_buffer, or @ref triangle_count if no draw buffer is used.
		 */
		resource_handle index_buffer = tz::nullhand;
		/**
		 * @brief Buffer containing an initial count and draw commands which may or may not be used in a single invocation of the pass.
		 *
		 * The memory contents of the draw buffer are expected to be:
		 * 
		 * `{drawcount, draws...}`
		 *
		 * Where `drawcount` is a unsigned 32-bit integer representing the number of proceeding draws which will occur in a single invocation of the pass.
		 * Following the draw count, the buffer should contain a tightly-packed array of draw commands.
		 * - If an index buffer has been provided (i.e @ref index_buffer is not null), then a draw command should be a @ref draw_indexed_t. Otherwise, a draw command should be a @ref draw_t.
		 * - The drawcount does not have to match the number of draw commands proceeding it in the buffer. If the drawcount is less than the number of draw commands, than the excess commands will not occur. If the drawcount is greater than the number of draw commands, then the entire array of draw commands will safely be used without any buffer overreads.
		 */
		resource_handle draw_buffer = tz::nullhand;
		/// Describe which faces will be culled during rendering.
		cull culling = cull::back;
		/// Specifies extra optional behaviour for the pass.
		graphics_flag flags = static_cast<graphics_flag>(0);
		/**
		 * @brief Number of triangles to draw in a single invocation of the pass.
		 *
		 * The count you specify affects how many vertices are drawn every frame. You can change this count later on via @ref pass_set_triangle_count.
		 *
		 * If you have specified an @ref index_buffer, then `3*triangle_count` indices will be drawn, sourced from the start of the index buffer.
		 *
		 * If you have specified a @ref draw_buffer, then this field is entirely ignored and the draw buffer becomes the only source of truth.
		 */
		std::size_t triangle_count = 0;
	};

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Specifies creation flags for a new compute pass.
	 *
	 * See @ref tz::gpu::pass_info for usage.
	 **/
	struct pass_compute_state
	{
		/// Number of workgroups (XYZ) to dispatch every frame.
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
		/**
		 * @brief List of all resources used in the shader program.
		 * 
		 * There are several restrictions you must adhere to when defining a set of resources:
		 * - You must not pass @ref tz::nullhand or @ref tz::gpu::window_resource.
		 * - You must pass a valid image or buffer resource, that is, the result of a previous call to @ref create_buffer or @ref create_image that you have not since destroyed via @ref destroy_resource.
		 *
		 * The order in which you provide the list of resources may affect the buffer resource index you need to use in the shader.
		 * - The id of the buffer is unaffected by any preceding images, and the id of the image is unaffected by any preceding buffers. This means you are free to intersperse as many images between buffers as you want (and vice-versa), without affecting the buffer ids.
		 * For example:
		 * `.resources = {myimage0, mybuffer0, myimage1, mybuffer1, myimage2, myimage3, mybuffer2}`
		 * Note that the number at the end of each resource name in the above example represents its buffer/image id used in the shader, i.e:
		 * `resource(id = 1) const buffer mybuffer1{...};`
		 * `vec4 myimage1_colour = sample(1, some_uv);`
		 */
		std::span<const resource_handle> resources = {};
		/// Specifies extra optional behaviour for the pass.
		pass_flag flags = static_cast<pass_flag>(0);
		/// Name identifying the pass. Debug tools/error checking will refer to the pass by this name if you provide one. If you do not provide a name, one will be automatically generated for you.
		const char* name = "<untitled pass>";
	};

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Corresponds to a previously-created pass.
	 */
	using pass_handle = tz::handle<pass_info>;

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Meta-pass that acts as an action to present the system image to the screen.
	 */
	constexpr auto present_pass = static_cast<tz::hanval>(std::numeric_limits<std::underlying_type_t<tz::hanval>>::max() - 1);

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Create a new pass.
	 * @return On success: A @ref pass_handle corresponding to the newly created pass.
	 * @return @ref tz::error_code::invalid_value If you fail to provide a valid shader program.
	 * @return @ref tz::error_code::invalid_value If you provide a malformed shader program. A shader program is only well-formed if it consists of only a compute shader, OR it consists of a single vertex and fragment shader.
	 * @return @ref tz::error_code::invalid_value For a graphics pass if you fail to provide at least one colour target.
	 * @return @ref tz::error_code::invalid_value For a graphics pass if you provide a colour target that is invalid. A valid colour target is either a.) the window resource (and you have opened a window), b.) an image resource created with @ref tz::gpu::image_flag::colour_target
	 * @return @ref tz::error_code::precondition_failure For a graphics pass if any colour target provided does not exactly match the dimensions of all other provided colour targets. All colour targets must be images with the same dimensions. This does mean that if you provide the window resource as a colour target, all other colour targets must have the same dimensions as the window.
	 * @return @ref tz::error_code::machine_unsuitable If the currently-used hardware does not support the pass you're attempting to create, for example if you attempt to create a graphics pass but your hardware has @ref hardware_capabilities::compute_only.
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
	 * @brief Set the triangle count of an existing graphics pass.
	 * @param graphics_pass Graphics pass to target. If you provide a compute pass, nothing interesting happens.
	 * @param triangle_count New number of triangles to render every frame.
	 *
	 * When you created a graphics pass, you set an initial triangle count via @ref pass_graphics_state::triangle_count. This function will override that count, meaning the next time a pass submits GPU work, the new number of triangles will be rendered.
	 *
	 * There are no GPU-sync considerations involved when calling this function.
	 * @warning If you fail to pass a valid @ref pass_handle to this function, the behaviour is undefined.
	 */
	void pass_set_triangle_count(pass_handle graphics_pass, std::size_t triangle_count);

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Set the compute kernel of an existing compute pass.
	 * @param compute_pass Compute pass to target. If you provide a graphics pass, nothing interesting happens.
	 * @param kernel New workgroup dimensions to be dispatched every frame.
	 *
	 * When you created a compute pass, you set an initial kernel size via @ref pass_compute_state::kernel. This function will override those dimensions, meaning the next time a pass submits GPU work, the new workgroup dimensions will be dispatched.
	 *
	 * There are no GPU-sync considerations involved when calling this function.
	 * @warning If you fail to pass a valid @ref pass_handle to this function, the behaviour is undefined.
	 */
	void pass_set_kernel(pass_handle compute_pass, tz::v3u kernel);

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Set the scissor rectangle of an existing graphics pass.
	 * @param graphics_pass Graphics pass to target. If you provide a graphics pass, nothing interesting happens.
	 * @param scissor Scissor rectangle, see @ref pass_graphics_state::scissor for further details.
	 *
	 * When you created a graphics pass, you may or may not have explicitly set an initial scissor rectangle via @ref pass_graphics_state::scissor. This function will override those dimensions, meaning the next time a pass submits GPU work, the new scissor rectangle will be used.
	 *
	 * There are no GPU-sync considerations involved when calling this function.
	 * @warning If you fail to pass a valid @ref pass_handle to this function, the behaviour is undefined.
	 */
	void pass_set_scissor(pass_handle graphics_pass, tz::v4u scissor);

	/**
	 * @ingroup tz_gpu_pass
	 * @brief Add a new image resource to be used in the pass.
	 * @return @ref tz::error_code::invalid_value If you fail to provide a valid image resource. Note that this excludes buffer resources, the window resource, and the null resource.
	 * @return @ref tz::error_code::driver_hazard If there are too many images used by the pass already. Passes have an implementation-defined maximum image count that is guaranteed to be at least 4096.
	 *
	 * This will permanently increase the number of resources used in the pass. It is not possible to change the shader used by a pass, so the shader associated with the pass is expected to conditionally use the new image id anyway.
	 *
	 * All other resources used by the pass are unchanged. The index of this new image to be used in the shader will be equal to the previous number of images.
	 */
	tz::error_code pass_add_image_resource(pass_handle pass, resource_handle res);

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