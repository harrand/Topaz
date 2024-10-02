#ifndef TOPAZ_GPU_SHADER_HPP
#define TOPAZ_GPU_SHADER_HPP
#include "tz/core/handle.hpp"
#include "tz/core/error.hpp"
#include <string>
#include <expected>

namespace tz::gpu
{
	struct shader_tag_t{};
	using shader_handle = tz::handle<shader_tag_t>;
	std::expected<shader_handle, tz::error_code> create_graphics_shader(std::string vertex_source, std::string fragment_source);
	std::expected<shader_handle, tz::error_code> create_compute_shader(std::string compute_source);
	void destroy_shader(shader_handle);
}

#endif // TOPAZ_GPU_SHADER_HPP