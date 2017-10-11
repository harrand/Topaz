#ifndef SHADER_HPP
#define SHADER_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <string>
#include "utility.hpp"
#include "glew.h"

namespace tz
{
	namespace graphics
	{
		// vertex, tessellation_control, tessellation_evaluation, geometry, fragment
		constexpr std::size_t maximum_shaders = 5;
	}
}

class Shader
{
public:
	Shader(std::string filename, bool compile = true, bool link = true, bool validate = true);
	Shader(const Shader& copy);
	Shader(Shader&& move);
	~Shader();
	Shader& operator=(const Shader& rhs) = delete;
	
	void compile();
	void link();
	void validate();
	bool is_compiled() const;
	bool is_linked() const;
	bool is_validated() const;
	bool ready() const;
	void initialise_uniforms();
	bool has_vertex_shader() const;
	bool has_tessellation_control_shader() const;
	bool has_tessellation_evaluation_shader() const;
	bool has_geometry_shader() const;
	bool has_fragment_shader() const;
	GLuint get_program_handle() const;
	void bind() const;
	void update(const std::array<float, 16>& model_matrix_array, const std::array<float, 16>& view_matrix_array, const std::array<float, 16>& projection_matrix_array, unsigned int shininess, float parallaxmap_scale_constant, float parallaxmap_offset_constant, float displacement_factor) const;
private:
	static std::string load_shader(const std::string& filename);
	static void check_shader_error(GLuint shader, GLuint flag, bool is_program, std::string error_message);
	static GLuint create_shader(std::string source, GLenum shader_type);
	enum class UniformTypes : std::size_t
	{
		MODEL,
		VIEW,
		PROJECTION,
		SHININESS,
		PARALLAX_MAP_SCALE,
		PARALLAX_MAP_BIAS,
		DISPLACEMENT_FACTOR,
		NUM_UNIFORMS,
	};
	std::string filename;
	bool compiled;
	GLuint program_handle;
	std::array<GLuint, tz::graphics::maximum_shaders> shaders;
	std::array<GLint, static_cast<std::size_t>(UniformTypes::NUM_UNIFORMS)> uniforms;
};
#endif