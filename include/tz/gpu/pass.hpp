#ifndef TOPAZ_GPU_PASS_HPP
#define TOPAZ_GPU_PASS_HPP

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
}

#endif // TOPAZ_GPU_PASS_HPP