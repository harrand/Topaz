#ifndef SHADER_HPP
#define SHADER_HPP
#include "utility/string.hpp"
#include "platform_specific/uniform.hpp"
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
		constexpr std::size_t specular_map_sampler_id = 4;
        constexpr std::size_t emissive_map_sampler_id = 5;
		// Allow a few more ids for extra assets which could appear in the future.
		constexpr std::size_t initial_extra_texture_sampler_id = 16;

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

#ifdef TOPAZ_DEBUG
namespace tz::platform
{
/**
* Use this to load, compile, link, run, edit, analyse and even receive transform-feedback from an OpenGL shader written in GLSL.
*/
	class OGLShader
	{
	public:
		/**
         * Constructs a shader from the given shader-sources.
         * Invalid shader sources will emit errors via tz::util::log::error.
         * Valid shader sources will yield desired shaders.
         * Empty shader sources will yield a lack of corresponding shaders (e.g if geometry_source is "", there shall be no geometry shader).
         * @param vertex_source - Source of the Vertex OGLShader
         * @param tessellation_control_source - Source of the Tessellation-Control OGLShader
         * @param tessellation_evaluation_source - Source of the Tessellation-Evaluation OGLShader
         * @param geometry_source - Source of the Geometry OGLShader
         * @param fragment_source - Source of the Fragment OGLShader
         * @param compile - Whether to instantly compile the OGLShader sources
         * @param link - Whether to instantly link the compiled OGLShader objects
         * @param validate - Whether to instantly validate the linked OGLShader program
         */
		OGLShader(std::string vertex_source, std::string tessellation_control_source,
				  std::string tessellation_evaluation_source, std::string geometry_source, std::string fragment_source,
				  bool compile = true, bool link = true, bool validate = true);
		/**
         * Constructs a shader from a given filename, comprised of:
         * Vertex shader from the path 'filename.vertex.glsl',
         * Tessellation-Control shader from the path 'filename.tessellation_control.glsl',
         * Tessellation-Evaluation shader from the path 'filename.tessellation_evaluation.glsl',
         * Geometry shader from the path 'filename.geometry.glsl',
         * and Fragment shader from the path 'filename.fragment.glsl'.
         * @param filename - Base filename of the OGLShader
         * @param compile - Whether to instantly compile the OGLShader sources
         * @param link - Whether to instantly link the compiled OGLShader objects
         * @param validate - Whether to instantly validate the linked OGLShader program
         */
		OGLShader(std::string filename, bool compile = true, bool link = true, bool validate = true);
		/**
         * Constructs a OGLShader from an existing OGLShader.
         * @param copy - The existing OGLShader to copy from
         */
		OGLShader(const OGLShader &copy);
		/**
         * Construct a OGLShader from an existing OGLShader.
         * @param move - The existing OGLShader to move from
         */
		OGLShader(OGLShader &&move);
		/**
         * Dispose of the OpenGL handle memory.
         */
		~OGLShader();
		/**
         * Until transform feedback and other compilation options are implemented, the idea of copy-constructing OGLShaders is meaningless.
         * @param rhs - N/A
         * @return - N/A
         */
		OGLShader &operator=(const OGLShader &rhs) = delete;
		/**
         * Compile the OGLShader sources
         * @param vertex_source - Source of the Vertex OGLShader
         * @param tessellation_control_source - Source of the Tessellation-Control OGLShader
         * @param tessellation_evaluation_source - Source of the Tessellation-Evaluation OGLShader
         * @param geometry_source - Source of the Geometry OGLShader
         * @param fragment_source - Source of the Fragment OGLShader
         */
		void compile(std::string vertex_source, std::string tessellation_control_source,
					 std::string tessellation_evaluation_source, std::string geometry_source,
					 std::string fragment_source);
		/**
         * Link the compiled OGLShader objects.
         */
		void link();
		/**
         * Validate the linked OGLShader program.
         */
		void validate();
		/**
         * Query whether the OGLShader sources have been compiled at least once.
         * @return - True if compilation has taken place. Otherwise false
         */
		bool is_compiled() const;
		/**
         * Query whether the compiled OGLShader objects have been linked together at least once.
         * @return - True if linkage has taken place. Otherwise false
         */
		bool is_linked() const;
		/**
         * Query whether the linked OGLShader program has been validated at least once.
         * @return - True if validation has taken place. Otherwise false
         */
		bool is_validated() const;
		/**
         * Query whether the OGLShader has been compiled, linked & validated and is therefore ready to be bound.
         * @return - True if the OGLShader is ready for use. Otherwise false
         */
		bool ready() const;
		/**
         * Add an existing Uniform to this OGLShader.
         * @tparam T - Type of the Uniform
         * @param uniform - The existing Uniform object
         */
		template<class T>
		void add_uniform(Uniform<T> &&uniform);
		/**
         * Construct a Uniform in-place in this OGLShader.
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
         * Query whether this OGLShader has a uniform with uniform-location equal to the parameter.
         * @param uniform_location - Desired uniform-location to query
         * @return - True if a Uniform with this location exists. Otherwise false
         */
		bool has_uniform(std::string_view uniform_location) const;
		/**
         * Get a pointer to the Uniform with uniform-location equal to the parameter.
         * @param uniform_location - Desired uniform-location to query
         * @return - Pointer to the Uniform. If it doesn't exist, nullptr is returned
         */
		UniformImplicit *get_uniform(std::string_view uniform_location) const;
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
         * Retrieve the number of Uniforms currently being utilised by this OGLShader.
         * @return - Number of Uniforms in this OGLShader
         */
		std::size_t number_active_uniforms() const;
		/**
         * Get the name of the attribute with this ID.
         * @param attribute_id - ID of the attribute to query
         * @return - Name of the attribute
         */
		const std::string &get_attribute_location(std::size_t attribute_id) const;
		/**
         * Register an attribute via name and ID.
         * @param attribute_id - Desired ID of the attribute
         * @param attribute_location - Location (name) of the attribute
         */
		void register_attribute(std::size_t attribute_id, std::string attribute_location);
		/**
         * Query whether this OGLShader has a valid Vertex component.
         * @return - True if the Vertex OGLShader component is valid. False otherwise
         */
		bool has_vertex_shader() const;
		/**
         * Query whether this OGLShader has a valid Tessellation-Control component.
         * @return - True if the Tessellation-Control OGLShader component is valid. False otherwise
         */
		bool has_tessellation_control_shader() const;
		/**
         * Query whether this OGLShader has a valid Tessellation-Evaluation component.
         * @return - True if the Tessellation-Evaluation OGLShader component is valid. False otherwise
         */
		bool has_tessellation_evaluation_shader() const;
		/**
         * Query whether this OGLShader has a valid Geometry component.
         * @return - True if the Geometry OGLShader component is valid. False otherwise
         */
		bool has_geometry_shader() const;
		/**
         * Query whether this OGLShader has a valid Fragment component.
         * @return - True if the Fragment OGLShader component is valid. False otherwise
         */
		bool has_fragment_shader() const;
		/**
         * Get the underlying OpenGL handle for this OGLShader program.
         * @return - OpenGL OGLShader program handle
         */
		GLuint get_program_handle() const;
		/**
         * Bind this OGLShader.
         */
		void bind() const;
		/**
         * Update and push all Uniforms into VRAM.
         */
		void update() const;
	private:
		/**
         * Print out a OGLShader error message if there is an error.
         * @param shader - OGLShader program handle to check
         * @param flag - What type of error to check for
         * @param is_program - Whether the OGLShader is a whole program or not
         * @param error_message - The error message prefix to print if there is an error
         */
		static void check_shader_error(GLuint shader, GLuint flag, bool is_program, std::string error_message);
		/**
         * Upload the specified OGLShader source to VRAM and compile it.
         * @param source - Source of the OGLShader
         * @param shader_type - Type of the OGLShader (e.g GL_VERTEX_SHADER)
         * @return - The shader-handle
         */
		static GLuint create_shader(std::string source, GLenum shader_type);

		/// Path to the external file used to load this OGLShader.
		std::string filename;
		/// Stores whether the OGLShader has been compiled or not.
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
	* Factory functions for OGLShaders that need no special source code; simply pass-through shaders and the ability to render 3D geometry, with some plain old textures.
	* If your application is so simple that only the simplest matrix transformations are needed with no fancy effects, use this.
	*/
	namespace tz::graphics::shader
	{
		/**
         * Create a pass-through OGLShader with some defaulted names.
         * @param position_attribute_name - Name of the position attribute
         * @param texture_coordinate_attribute_name - Name of the texture-coordinate attribute
         * @param texture_sampler_name - Name of the texture-sampler attribute
         * @return - Constructed OGLShader object
         */
		OGLShader pass_through(std::string position_attribute_name = "position_modelspace_attribute", std::string texture_coordinate_attribute_name = "texture_coordinate_attribute", std::string texture_sampler_name = "texture_sampler_uniform");
	}
}

using Shader = tz::platform::OGLShader;
#endif
#include "shader.inl"
#endif