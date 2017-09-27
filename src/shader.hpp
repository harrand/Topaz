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
		constexpr unsigned int maximum_shaders = 5;
	}
}

class Shader
{
public:
	Shader(std::string filename);
	Shader(const Shader& copy);
	Shader(Shader&& move);
	~Shader();
	Shader& operator=(const Shader& rhs) = delete;
	
	bool hasVertexShader() const;
	bool hasTessellationControlShader() const;
	bool hasTessellationEvaluationShader() const;
	bool hasGeometryShader() const;
	bool hasFragmentShader() const;
	GLuint getProgramHandle() const;
	void bind() const;
	void update(const std::array<float, 16>& model_matrix_array, const std::array<float, 16>& view_matrix_array, const std::array<float, 16>& projection_matrix_array, unsigned int shininess, float parallaxmap_scale_constant, float parallaxmap_offset_constant, float displacement_factor) const;
private:
	static std::string loadShader(const std::string& filename);
	static void checkShaderError(GLuint shader, GLuint flag, bool is_program, std::string error_message);
	static GLuint createShader(std::string source, GLenum shader_type);
	enum class UniformTypes : unsigned int
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
	GLuint program_handle;
	GLuint shaders[tz::graphics::maximum_shaders];
	GLuint uniforms[static_cast<std::size_t>(UniformTypes::NUM_UNIFORMS)];
};
#endif