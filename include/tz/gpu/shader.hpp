#ifndef TOPAZ_GPU_SHADER_HPP
#define TOPAZ_GPU_SHADER_HPP
#include "tz/core/handle.hpp"
#include "tz/core/error.hpp"
#include <string>
#include <expected>

namespace tz::gpu
{
	struct shader_tag_t{};
	/**
	 * @ingroup tz_gpu
	 * @defgroup tz_gpu_shader Shaders
	 * @brief Create shaders to carry out programmable GPU work.
	 **/

	/**
	 * @ingroup tz_gpu_shader
	 * @brief Corresponds to a previously-created shader program (for graphics or compute).
	 */
	using shader_handle = tz::handle<shader_tag_t>;
	/**
	 * @ingroup tz_gpu_shader
	 * @brief Create a new shader intended for graphics GPU work, comprised of a vertex and fragment shader.
	 * @param vertex_source Code for the vertex shader, in SPIRV. 
	 * @param fragment_source Code for the fragment shader, in SPIRV. 
	 * @return On success: A @ref shader_handle corresponding to the newly-created shader program.
	 * @return @ref tz::error_code::oom If CPU memory is exhausted whilst trying to create the shader program.
	 * @return @ref tz::error_code::voom If GPU memory is exhausted whilst trying to create the shader program.
	 * @return @ref tz::error_code::unknown_error If some other undocumented error occurs.
	 **/
	std::expected<shader_handle, tz::error_code> create_graphics_shader(std::string vertex_source, std::string fragment_source);
	/**
	 * @ingroup tz_gpu_shader
	 * @brief Create a new shader intended for compute GPU work, comprised of a single compute shader.
	 * @param compute_source Code for the compute shader, in SPIRV. 
	 * @return On success: A @ref shader_handle corresponding to the newly-created shader program.
	 * @return @ref tz::error_code::oom If CPU memory is exhausted whilst trying to create the shader program.
	 * @return @ref tz::error_code::voom If GPU memory is exhausted whilst trying to create the shader program.
	 * @return @ref tz::error_code::unknown_error If some other undocumented error occurs.
	 **/
	std::expected<shader_handle, tz::error_code> create_compute_shader(std::string compute_source);
	/**
	 * @ingroup tz_gpu_shader
	 * @brief Manually destroy a previously-created shader.
	 *
	 * If you are certain that you are done with a shader program, you can destroy it here, causing all of its memory to be available for reuse.
	 *
	 * @note If you never destroy a shader manually, it will automatically be destroyed for you when you call @ref tz::terminate.
	 *
	 * @warning Be aware of GPU synchronisation! Ensure that you do not destroy a shader that is currently in-use by ongoing GPU work. Take care to ensure that renderers using the resource have finished their work before tearing it down.
	 **/
	void destroy_shader(shader_handle);
}

#endif // TOPAZ_GPU_SHADER_HPP