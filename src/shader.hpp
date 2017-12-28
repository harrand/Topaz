#ifndef SHADER_HPP
#define SHADER_HPP
#include <string>
#include <memory>
#include <string_view>
#include <unordered_set>
#include "utility.hpp"
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "GL/glew.h"

namespace tz
{
	namespace graphics
	{
		// vertex, tessellation_control, tessellation_evaluation, geometry, fragment
		constexpr std::size_t maximum_shaders = 5;
		constexpr std::size_t maximum_uniforms = GL_MAX_UNIFORM_LOCATIONS;
	}
	namespace util
	{
		// Get a label for a shader_type (e.g GL_FRAGMENT_SHADER returns "Fragment")
		const char* shader_type_string(GLenum shader_type);
	}
}

/*
	Abstract. Not available for non-polymorphic use. I highly doubt you'll need to inherit from this anyway. Consider providing template specialisation to Uniform<T> if you need custom uniforms, which you may need in a later version of OpenGL which supports more primitives, perhaps.
*/
class UniformImplicit
{
public:
	virtual GLuint get_shader_handle() const = 0;
	virtual std::string_view get_uniform_location() const = 0;
	virtual void push() const = 0;
};

/*
	Represent an OpenGL uniform in C++. Supports the following Topaz/C++ primitives:
	bool, int, unsigned int, float, double, Vector2F, Vector3F, Vector4F, Matrix2x2, Matrix3x3, Matrix4x4
	If the template argument is not any of these types, a static assertation will fail in Uniform<T>::push and emit a compiler error.
*/
template<class T>
class Uniform : public UniformImplicit
{
public:
	Uniform<T>(GLuint shader_handle, std::string uniform_location, T value);
	Uniform<T>(const Uniform<T>& copy) = delete;	// Delete copy ctor because no uniform_location can have more than one value.
	Uniform<T>(Uniform<T>&& move) = default;
	~Uniform<T>() = default;
	
	GLuint get_shader_handle() const;
	virtual std::string_view get_uniform_location() const final;
	const T& get_value() const;
	void set_value(T value);
	virtual void push() const final;
private:
	GLuint shader_handle;
	std::string uniform_location;
	T value;
	GLint uniform_handle;
};

/*
	Use this to load, compile, link, run, edit, analyse and even receive transform-feedback from an OpenGL shader written in GLSL.
*/
class Shader
{
public:
	Shader(std::string vertex_source, std::string tessellation_control_source, std::string tessellation_evaluation_source, std::string geometry_source, std::string fragment_source, bool compile = true, bool link = true, bool validate = true);
	Shader(std::string filename, bool compile = true, bool link = true, bool validate = true);
	Shader(const Shader& copy);
	Shader(Shader&& move);
	~Shader();
	Shader& operator=(const Shader& rhs) = delete;
	
	void compile(std::string vertex_source, std::string tessellation_control_source, std::string tessellation_evaluation_source, std::string geometry_source, std::string fragment_source);
	void link();
	void validate();
	bool is_compiled() const;
	bool is_linked() const;
	bool is_validated() const;
	bool ready() const;
	template<class T>
	void add_uniform(Uniform<T>&& uniform);
	template<class T>
	void emplace_uniform(std::string uniform_location, T value);
	void remove_uniform(std::string_view uniform_location);
	bool has_uniform(std::string_view uniform_location) const;
	UniformImplicit* get_uniform(std::string_view uniform_location) const;
	template<class T>
	void set_uniform(std::string_view uniform_location, T value);
	template<class T>
	T get_uniform_value(std::string_view uniform_location) const;
	std::size_t number_active_uniforms() const;
	bool has_vertex_shader() const;
	bool has_tessellation_control_shader() const;
	bool has_tessellation_evaluation_shader() const;
	bool has_geometry_shader() const;
	bool has_fragment_shader() const;
	GLuint get_program_handle() const;
	void bind() const;
	void update() const;
private:
	static void check_shader_error(GLuint shader, GLuint flag, bool is_program, std::string error_message);
	static GLuint create_shader(std::string source, GLenum shader_type);
	std::string filename;
	bool compiled;
	GLuint program_handle;
	std::array<GLuint, tz::graphics::maximum_shaders> shaders;
	std::array<std::unique_ptr<UniformImplicit>, tz::graphics::maximum_uniforms> uniform_data;
	std::size_t uniform_counter;
};

namespace tz::graphics::shader
{
	Shader pass_through(std::string position_attribute_name = "position_modelspace_attribute", std::string texture_coordinate_attribute_name = "texture_coordinate_attribute", std::string texture_sampler_name = "texture_sampler_uniform");
}
#include "shader.inl"
#endif