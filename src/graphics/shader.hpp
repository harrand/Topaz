#ifndef SHADER_HPP
#define SHADER_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "GL/glew.h"
#include "utility/string.hpp"
#include "graphics/light.hpp"
#include <string>
#include <memory>
#include <string_view>
#include <unordered_set>
#include <map>

namespace tz
{
	namespace graphics
	{
		constexpr std::size_t texture_sampler_id = 0;
		constexpr std::size_t texture_cubemap_sampler_id = 0;
		constexpr std::size_t normal_map_sampler_id = 1;
		constexpr std::size_t parallax_map_sampler_id = 2;
		constexpr std::size_t displacement_map_sampler_id = 3;

        constexpr char render_shader_model_uniform_name[] = "m";
        constexpr char render_shader_view_uniform_name[] = "v";
        constexpr char render_shader_projection_uniform_name[] = "p";
		/**
		* Vertex, Tessellation Control, Tessellation Evaluation, Geometry, Fragment.
		*/
		constexpr std::size_t maximum_shaders = 5;
		constexpr std::size_t maximum_uniforms = GL_MAX_UNIFORM_LOCATIONS;
	}
	namespace util
	{
		/**
		* Get a label for a shader_type (e.g GL_FRAGMENT_SHADER returns "Fragment")
		*/
		const char* shader_type_string(GLenum shader_type);
	}
}

/**
* Abstract. Not available for non-polymorphic use.
* I highly doubt you'll need to inherit from this anyway.
* Consider providing template specialisation to Uniform<T> if you need custom uniforms, which you may need in a later version of OpenGL which supports more primitives, perhaps.
*/
class UniformImplicit
{
public:
	UniformImplicit(GLuint shader_handle, std::string uniform_location);
	virtual GLuint get_shader_handle() const;
	virtual std::string_view get_uniform_location() const;
	/**
	 * Pure Virtual
	 */
	virtual void push() const = 0;
protected:
	/// Underlying OpenGL shader-handle.
	GLuint shader_handle;
	/// OpenGL uniform-location (name of the variable in GLSL).
	std::string uniform_location;
};

/**
 * Represent an OpenGL uniform in C++. Supports the following Topaz/C++ primitives:
 * bool, int, unsigned int, float, double, Vector2F, Vector3F, Vector4F, Matrix2x2, Matrix3x3, and Matrix4x4.
 * If the template argument is not any of these types, a static assertation will fail in Uniform<T>::push and emit a compiler error.
 * @tparam T - Type of the Uniform value
 */
template<class T>
class Uniform : public UniformImplicit
{
public:
	/**
	 * Construct a Uniform from all specifications.
	 * @param shader_handle - The OpenGL shader-handle referring to the Shader which this Uniform should belong to
	 * @param uniform_location - OpenGL uniform-location (i.e name of the Uniform)
	 * @param value - Value of the Uniform
	 */
	Uniform<T>(GLuint shader_handle, std::string uniform_location, T value);
	/**
	 * Uniforms are not copyable.
	 * @param copy - N/A
	 */
	Uniform<T>(const Uniform<T>& copy) = delete;
	/**
	 * Construct a Uniform from an existing Uniform.
	 * @param move - The existing Uniform to move from.
	 */
	Uniform<T>(Uniform<T>&& move) = default;
	/**
	 * Read-only access to the underlying Uniform's value.
	 * @return - Value of the Uniform
	 */
	const T& get_value() const;
	/**
	 * Assign the value of the Uniform.
	 * @param value - Desired new value of the Uniform
	 */
	void set_value(T value);
	/**
	 * Update all Uniform changes and have them affect all subsequent render-passes.
	 */
	virtual void push() const final;
private:
	/// Underlying value.
	T value;
	/// Underlying OpenGL handle for this Uniform.
	GLint uniform_handle;
};

template<>
class Uniform<DirectionalLight> : public UniformImplicit
{
public:
	/**
	 * Construct a DirectionalLight Uniform from all specifications.
	 * @param shader_handle - The OpenGL shader-handle referring to the Shader which this Uniform should belong to
	 * @param uniform_location - OpenGL uniform-location (i.e name of the Uniform)
	 * @param value - Value of the Uniform
	 */
	Uniform<DirectionalLight>(GLuint shader_handle, std::string uniform_location, DirectionalLight value);
	/**
	 * Uniforms are not copyable.
	 * @param copy - N/A
	 */
	Uniform<DirectionalLight>(const Uniform<DirectionalLight>& copy) = delete;
	/**
	 * Construct a Uniform from an existing Uniform.
	 * @param move - The existing Uniform to move from.
	 */
	Uniform<DirectionalLight>(Uniform<DirectionalLight>&& move) = default;
	/**
	 * Read-only access to the underlying Uniform's value.
	 * @return - Value of the Uniform
	 */
	const DirectionalLight& get_value() const;
	/**
	 * Assign the value of the Uniform.
	 * @param value - Desired new value of the Uniform
	 */
	void set_value(DirectionalLight value);
	/**
	 * Update all Uniform changes and have them affect all subsequent render-passes.
	 */
	virtual void push() const final;
private:
	/// Underlying value.
	DirectionalLight value;
	/// DirectionalLight is (direction, colour, power) so each needs their own uniform.
	GLint direction_uniform_handle;
	GLint colour_uniform_handle;
	GLint power_uniform_handle;
};

template<>
class Uniform<PointLight> : public UniformImplicit
{
public:
	/**
	 * Construct a PointLight Uniform from all specifications.
	 * @param shader_handle - The OpenGL shader-handle referring to the Shader which this Uniform should belong to
	 * @param uniform_location - OpenGL uniform-location (i.e name of the Uniform)
	 * @param value - Value of the Uniform
	 */
	Uniform<PointLight>(GLuint shader_handle, std::string uniform_location, PointLight value);
	/**
	 * Uniforms are not copyable.
	 * @param copy - N/A
	 */
	Uniform<PointLight>(const Uniform<PointLight>& copy) = delete;
	/**
	 * Construct a Uniform from an existing Uniform.
	 * @param move - The existing Uniform to move from.
	 */
	Uniform<PointLight>(Uniform<PointLight>&& move) = default;
	/**
	 * Read-only access to the underlying Uniform's value.
	 * @return - Value of the Uniform
	 */
	const PointLight& get_value() const;
	/**
	 * Assign the value of the Uniform.
	 * @param value - Desired new value of the Uniform
	 */
	void set_value(PointLight value);
	/**
	 * Update all Uniform changes and have them affect all subsequent render-passes.
	 */
	virtual void push() const final;
private:
	/// Underlying value.
	PointLight value;
	/// DirectionalLight is (direction, colour, power) so each needs their own uniform.
	GLint position_uniform_handle;
	GLint colour_uniform_handle;
	GLint power_uniform_handle;
};

/**
* Use this to load, compile, link, run, edit, analyse and even receive transform-feedback from an OpenGL shader written in GLSL.
*/
class Shader
{
public:
	/**
	 * Constructs a shader from the given shader-sources.
	 * Invalid shader sources will emit errors via tz::util::log::error.
	 * Valid shader sources will yield desired shaders.
	 * Empty shader sources will yield a lack of corresponding shaders (e.g if geometry_source is "", there shall be no geometry shader).
	 * @param vertex_source - Source of the Vertex Shader
	 * @param tessellation_control_source - Source of the Tessellation-Control Shader
	 * @param tessellation_evaluation_source - Source of the Tessellation-Evaluation Shader
	 * @param geometry_source - Source of the Geometry Shader
	 * @param fragment_source - Source of the Fragment Shader
	 * @param compile - Whether to instantly compile the Shader sources
	 * @param link - Whether to instantly link the compiled Shader objects
	 * @param validate - Whether to instantly validate the linked Shader program
	 */
	Shader(std::string vertex_source, std::string tessellation_control_source, std::string tessellation_evaluation_source, std::string geometry_source, std::string fragment_source, bool compile = true, bool link = true, bool validate = true);
	/**
	 * Constructs a shader from a given filename, comprised of:
	 * Vertex shader from the path 'filename.vertex.glsl',
	 * Tessellation-Control shader from the path 'filename.tessellation_control.glsl',
	 * Tessellation-Evaluation shader from the path 'filename.tessellation_evaluation.glsl',
	 * Geometry shader from the path 'filename.geometry.glsl',
	 * and Fragment shader from the path 'filename.fragment.glsl'.
	 * @param filename - Base filename of the Shader
	 * @param compile - Whether to instantly compile the Shader sources
	 * @param link - Whether to instantly link the compiled Shader objects
	 * @param validate - Whether to instantly validate the linked Shader program
	 */
	Shader(std::string filename, bool compile = true, bool link = true, bool validate = true);
	/**
	 * Constructs a Shader from an existing Shader.
	 * @param copy - The existing Shader to copy from
	 */
	Shader(const Shader& copy);
	/**
	 * Construct a Shader from an existing Shader.
	 * @param move - The existing Shader to move from
	 */
	Shader(Shader&& move);
	/**
	 * Dispose of the OpenGL handle memory.
	 */
	~Shader();
	/**
	 * Until transform feedback and other compilation options are implemented, the idea of copy-constructing Shaders is meaningless.
	 * @param rhs - N/A
	 * @return - N/A
	 */
	Shader& operator=(const Shader& rhs) = delete;

	/**
	 * Compile the Shader sources
	 * @param vertex_source - Source of the Vertex Shader
	 * @param tessellation_control_source - Source of the Tessellation-Control Shader
	 * @param tessellation_evaluation_source - Source of the Tessellation-Evaluation Shader
	 * @param geometry_source - Source of the Geometry Shader
	 * @param fragment_source - Source of the Fragment Shader
	 */
	void compile(std::string vertex_source, std::string tessellation_control_source, std::string tessellation_evaluation_source, std::string geometry_source, std::string fragment_source);
	/**
	 * Link the compiled Shader objects.
	 */
	void link();
	/**
	 * Validate the linked Shader program.
	 */
	void validate();
	/**
	 * Query whether the Shader sources have been compiled at least once.
	 * @return - True if compilation has taken place. Otherwise false
	 */
	bool is_compiled() const;
	/**
	 * Query whether the compiled Shader objects have been linked together at least once.
	 * @return - True if linkage has taken place. Otherwise false
	 */
	bool is_linked() const;
	/**
	 * Query whether the linked Shader program has been validated at least once.
	 * @return - True if validation has taken place. Otherwise false
	 */
	bool is_validated() const;
	/**
	 * Query whether the Shader has been compiled, linked & validated and is therefore ready to be bound.
	 * @return - True if the Shader is ready for use. Otherwise false
	 */
	bool ready() const;
	/**
	 * Add an existing Uniform to this Shader.
	 * @tparam T - Type of the Uniform
	 * @param uniform - The existing Uniform object
	 */
	template<class T>
	void add_uniform(Uniform<T>&& uniform);
	/**
	 * Construct a Uniform in-place in this Shader.
	 * @tparam T - Type of the Uniform
	 * @param uniform_location - OpenGL uniform-location (name of the Uniform)
	 * @param value - Value of the Uniform
	 */
	template<class T>
	void emplace_uniform(std::string uniform_location, T value);
	/**
	 * Remove an existing Uniform equal to the uniform-location specified.
	 * @param uniform_location - OpenGL uniform-location of the Uniform to remove
	 */
	void remove_uniform(std::string_view uniform_location);
	/**
	 * Query whether this Shader has a uniform with uniform-location equal to the parameter.
	 * @param uniform_location - Desired uniform-location to query
	 * @return - True if a Uniform with this location exists. Otherwise false
	 */
	bool has_uniform(std::string_view uniform_location) const;
	/**
	 * Get a pointer to the Uniform with uniform-location equal to the parameter.
	 * @param uniform_location - Desired uniform-location to query
	 * @return - Pointer to the Uniform. If it doesn't exist, nullptr is returned
	 */
	UniformImplicit* get_uniform(std::string_view uniform_location) const;
	/**
	 * Assign a new value to an existing Uniform.
	 * @tparam T - Type of the Uniform
	 * @param uniform_location - OpenGL uniform-location whos value should change
	 * @param value - New value of the Uniform
	 */
	template<class T>
	void set_uniform(std::string_view uniform_location, T value);
	/**
	 * Get the value of an existing Uniform.
	 * @tparam T - Type of the Uniform
	 * @param uniform_location - OpenGL uniform-location whos value should be read
	 * @return - Value of the specified Uniform
	 */
	template<class T>
	T get_uniform_value(std::string_view uniform_location) const;
	/**
	 * Retrieve the number of Uniforms currently being utilised by this Shader.
	 * @return - Number of Uniforms in this Shader
	 */
	std::size_t number_active_uniforms() const;
	/**
	 * Get the name of the attribute with this ID.
	 * @param attribute_id - ID of the attribute to query
	 * @return - Name of the attribute
	 */
	const std::string& get_attribute_location(std::size_t attribute_id) const;
	/**
	 * Register an attribute via name and ID.
	 * @param attribute_id - Desired ID of the attribute
	 * @param attribute_location - Location (name) of the attribute
	 */
	void register_attribute(std::size_t attribute_id, std::string attribute_location);
	/**
	 * Query whether this Shader has a valid Vertex component.
	 * @return - True if the Vertex Shader component is valid. False otherwise
	 */
	bool has_vertex_shader() const;
	/**
	 * Query whether this Shader has a valid Tessellation-Control component.
	 * @return - True if the Tessellation-Control Shader component is valid. False otherwise
	 */
	bool has_tessellation_control_shader() const;
	/**
	 * Query whether this Shader has a valid Tessellation-Evaluation component.
	 * @return - True if the Tessellation-Evaluation Shader component is valid. False otherwise
	 */
	bool has_tessellation_evaluation_shader() const;
	/**
	 * Query whether this Shader has a valid Geometry component.
	 * @return - True if the Geometry Shader component is valid. False otherwise
	 */
	bool has_geometry_shader() const;
	/**
	 * Query whether this Shader has a valid Fragment component.
	 * @return - True if the Fragment Shader component is valid. False otherwise
	 */
	bool has_fragment_shader() const;
	/**
	 * Get the underlying OpenGL handle for this Shader program.
	 * @return - OpenGL Shader program handle
	 */
	GLuint get_program_handle() const;
	/**
	 * Bind this Shader.
	 */
	void bind() const;
	/**
	 * Update and push all Uniforms into VRAM.
	 */
	void update() const;
private:
	/**
	 * Print out a Shader error message if there is an error.
	 * @param shader - Shader program handle to check
	 * @param flag - What type of error to check for
	 * @param is_program - Whether the Shader is a whole program or not
	 * @param error_message - The error message prefix to print if there is an error
	 */
	static void check_shader_error(GLuint shader, GLuint flag, bool is_program, std::string error_message);
	/**
	 * Upload the specified Shader source to VRAM and compile it.
	 * @param source - Source of the Shader
	 * @param shader_type - Type of the Shader (e.g GL_VERTEX_SHADER)
	 * @return - The shader-handle
	 */
	static GLuint create_shader(std::string source, GLenum shader_type);

	/// Path to the external file used to load this Shader.
	std::string filename;
	/// Stores whether the Shader has been compiled or not.
	bool compiled;
	/// OpenGL program-handle.
	GLuint program_handle;
	/// Array of the shader-handles.
	std::array<GLuint, tz::graphics::maximum_shaders> shaders;
	/// Array of heap-stored Uniforms.
	std::array<std::unique_ptr<UniformImplicit>, tz::graphics::maximum_uniforms> uniform_data;
	/// Organised structure of attributes.
	std::map<GLuint, std::string> attribute_locations;
	/// Number of active Uniforms.
	std::size_t uniform_counter;
};

/**
* Factory functions for Shaders that need no special source code; simply pass-through shaders and the ability to render 3D geometry, with some plain old textures.
* If your application is so simple that only the simplest matrix transformations are needed with no fancy effects, use this.
*/
namespace tz::graphics::shader
{
	/**
	 * Create a pass-through Shader with some defaulted names.
	 * @param position_attribute_name - Name of the position attribute
	 * @param texture_coordinate_attribute_name - Name of the texture-coordinate attribute
	 * @param texture_sampler_name - Name of the texture-sampler attribute
	 * @return - Constructed Shader object
	 */
	Shader pass_through(std::string position_attribute_name = "position_modelspace_attribute", std::string texture_coordinate_attribute_name = "texture_coordinate_attribute", std::string texture_sampler_name = "texture_sampler_uniform");
}
#include "shader.inl"
#endif