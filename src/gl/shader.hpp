#ifndef TOPAZ_GL_SHADER_HPP
#define TOPAZ_GL_SHADER_HPP
#include "glad/glad.h"
#include <cstdint>
#include <array>
#include <optional>
#include <string>

namespace tz::gl
{
	class Texture;
}

namespace tz::gl
{
	enum class ShaderType : std::size_t
	{
		Vertex,
		TessellationControl,
		TessellationEvaluation,
		Geometry,
		Fragment,

		NUM_TYPES,
		Compute
	};

	/**
	 * Shader components which are attached to ShaderPrograms. Shaders can be one of several types, indicated by the ShaderType enum.
	 * 
	 * Shaders are written in GLSL. The shader source can be uploaded to this Shader component. Once that's done, compilation can be done.
	 * Compilation of a Shader requires an existing ShaderCompiler. Some compilers do the bare-minimum, others may include useful preprocessing directives (TZGLP).
	 */
	class Shader
	{
	public:
		/**
		 * Construct a Shader of the given type with no source.
		 * @param type Type of this Shader.
		 */
		Shader(ShaderType type);
		/**
		 * Construct a Shader of the given type, and instantly upload the source.
		 * @param type Type of this Shader.
		 * @param source Source-code of this Shader.
		 */
		Shader(ShaderType type, std::string source);
		/**
		 * Upload the shader source to the graphics driver, causing it to be used in subsequent compile invocations.
		 * 
		 * Note: This will discard previous compilation data, meaning that this will need to be compiled again even if it has been previously compiled successfully beforehand.
		 * @param source Source-code of the Shader, in GLSL.
		 */
		void upload_source(std::string source);
		/**
		 * Query as to whether some source-code has already been uploaded to this Shader.
		 * @return True if source has been uploaded. Otherwise false.
		 */
		bool has_source() const;
		/**
		 * Query as to whether some ShaderCompiler has already successfully compiled this Shader.
		 * @return True if a compiler has successfully compiled the Shader. Otherwise false.
		 */
		bool compiled() const;

		friend class ShaderCompiler; // Pretty much unavoidable tight-coupling, and I don't want to merge the two things.
		friend class ShaderProgram; // Same here. Hard to attach via our handle if we can't access it, and exposing the handle is not going to happen.
	private:
		void verify() const;

		ShaderType type;
		std::string source;
		GLuint handle;
		bool compilation_successful; // We always expect ShaderCompiler to mess with this.
	};
	using ShaderProgramHandle = GLuint;

	/**
	 * Represents a program designed to run on the GPU.
	 * 
	 * ShaderPrograms contain Shader components. Setting up ShaderPrograms is not a trivial matter.
	 * 
	 * To setup and use a ShaderProgram, you must:
	 * - Write shader sources in GLSL and upload the sources to various Shader components.
	 * - Compile these Shader components using a ShaderCompiler.
	 * - Ensure that a successfully compiled Vertex and Fragment shader are attached to this program.
	 * - Link this program via the same ShaderCompiler.
	 * - Ensure that the ShaderCompiler never gave off any erroneous diagnostics.
	 * 		- This will be true if ShaderProgram::usable() is now true.
	 * - Simply bind the program via ShaderProgram::bind(). Subsequent render-calls will now attempt to use this program.
	 */
	class ShaderProgram
	{
	public:
		/**
		 * Create an empty program with no Shader components attached.
		 */
		ShaderProgram();
		/// ShaderPrograms are non-copyable.
		ShaderProgram(const ShaderProgram& copy) = delete;
		ShaderProgram(ShaderProgram&& move);
		~ShaderProgram();
		/// ShaderPrograms are non-copyassignable.
		ShaderProgram& operator=(const ShaderProgram& rhs) = delete;
		ShaderProgram& operator=(ShaderProgram&& rhs);
		/**
		 * Create a Shader component in-place with the given ShaderType.
		 * 
		 * Note: If a Shader of this type already exists, this will leak memory.
		 * TODO: Improve this to allow this to overwrite existing Shader components of the same type.
		 * Note: Compute Shaders are not yet implemented. Attempting to emplace a Compute Shader will invoke UB without asserting.
		 * @tparam Args Types of the arguments used to construct the Shader.
		 * @param type Type of the Shader component to create (e.g a Vertex Shader).
		 * @param args Values of the arguments used to construct the Shader.
		 */
		template<typename... Args>
		Shader* emplace(ShaderType type, Args&&... args);
		void define(std::size_t index, const GLchar* name);
		/**
		 * Query as to whether this program is in a valid state to be linked. Programs are linkable if they contain at least a Vertex & Fragment shader and all attached shaders have been compiled successfully.
		 */
		bool linkable() const;
		/**
		 * Query as to whether this program is in a valid state to be bound. Programs are usable if they have been successfully linked.
		 */
		bool usable() const;
		/**
		 * Bind a program, causing its executable to be used by the GPU in all subsequent render invocations.
		 * 
		 * Precondition: The program must be usable. Otherwise, this will assert and invoke UB.
		 */
		void bind();

		/**
		 * Retrieve the maximum number of attached textures possible.
		 * @return Maximum number of possible attachments.
		 */
		std::size_t attached_textures_capacity() const;
		/**
		 * Retrieve the current number of attached textures.
		 * @return Number of attached textures.
		 */
		std::size_t attached_textures_size() const;
		/**
		 * Attach a texture to the given index. Only one texture can hold an index at a time.
		 * 
		 * Precondition: The texture provided must not reach the end of its lifetime until either a.) Another texture is attached in its place (or it is detached). b.) This ShaderProgram reaches the end of its lifetime. Otherwise, this will invoke UB without asserting.
		 * Precondition: idx must be in-range (idx < this->attached_textures_capacity()). Otherwise, this will assert and may invoke UB.
		 * @param idx The texture binding id to use.
		 * @param texture The texture to associate with the ID.
		 * @param sampler_name The expected name of the corresponding sampler2D in GLSL source.
		 */
		void attach_texture(std::size_t idx, const Texture* texture, std::string sampler_name);
		/**
		 * Detach the texture at the given index.
		 * 
		 * Precondition: idx must be in-range (idx < this->attached_textures_capacity()). Otherwise, this will assert and may invoke UB.
		 * @param idx The index corresponding to the texture to detach. If no texture was attached at the index, there is no effect.
		 */
		void detach_texture(std::size_t idx);
		/**
		 * Retrieve the texture at the given index.
		 * 
		 * Precondition: idx must be in-range (idx < this->attached_textures_capacity()). Otherwise, this will assert and may invoke UB.
		 * @param idx The index corresponding to the texture attachment to retrieve.
		 * @return Pointer to the attached texture if there is one at the given index. Otherwise nullptr.
		 */
		const Texture* get_attachment(std::size_t idx) const;

		/**
		 * Query as to whether a Shader component of the given type is currently attached to this program.
		 * 
		 * Precondition: The shader type must not be NUM_TYPES. Otherwise, this will assert and invoke UB.
		 * @param type Type of the shader whose existence should be queried. This must not be NUM_TYPES.
		 */
		bool has_shader(ShaderType type) const;
		bool operator==(ShaderProgramHandle rhs) const;
		bool operator!=(ShaderProgramHandle rhs) const;

		friend class ShaderCompiler; // Pretty much unavoidable tight-coupling, and I don't want to merge the two things.
	private:
		void verify() const;
		void nullify_all();
		void bind_textures() const;

		ShaderProgramHandle handle;
		std::array<std::optional<Shader>, static_cast<std::size_t>(ShaderType::NUM_TYPES)> shaders;
		std::array<const Texture*, static_cast<std::size_t>(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)> textures;
		std::array<std::string, static_cast<std::size_t>(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)> texture_names;
		bool ready;
	};

	namespace detail
	{
		constexpr GLenum resolve_type(ShaderType type);
	}

	namespace bound
	{
		ShaderProgramHandle shader_program();
	}
}

#include "gl/shader.inl"
#endif // TOPAZ_GL_SHADER_HPP