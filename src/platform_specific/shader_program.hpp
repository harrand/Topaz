//
// Created by Harrand on 05/04/2019.
//

#ifndef TOPAZ_SHADER_PROGRAM_HPP
#define TOPAZ_SHADER_PROGRAM_HPP

#include "platform_specific/vertex_buffer.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
    /**
     * Describes the type of a given component shader.
     */
    enum class OGLShaderComponentType : GLenum
    {
        VERTEX = GL_VERTEX_SHADER,
        TESSELLATION_CONTROL = GL_TESS_CONTROL_SHADER,
        TESSELLATION_EVALUATION = GL_TESS_EVALUATION_SHADER,
        GEOMETRY = GL_GEOMETRY_SHADER,
        FRAGMENT = GL_FRAGMENT_SHADER,
        COMPUTE = GL_COMPUTE_SHADER
    };

    class OGLShaderComponent;

    /**
     * Contains information about the results of a previous compilation attempt.
     */
    class OGLShaderComponentCompileResult
    {
    public:
        /**
         * Construct a result based upon the most recent compilation of a given component.
         * @param component - The component which was previously attempted to be compiled
         */
        OGLShaderComponentCompileResult(const OGLShaderComponent& component);
        /**
         * Query as to whether the compilation was successful or not.
         * @return - True if compilation was successful, otherwise false
         */
        bool was_successful() const;
        /**
         * Retrieve a pointer to the error-message, if there was one. If compilation was successful, then there shall be no message.
         * @return - Pointer to an error message if it exists, otherwise nullptr
         */
        const std::string* get_error_message() const;
        /**
         * Report the error message to the given std::ostream.
         * @param str - The stream via which to report the compilation error message
         */
        void report(std::ostream& str) const;
        /**
         * Invokes report(str) if the compilation result was a failure.
         * @param str - The stream via which to report the compilation error message
         */
        void report_if_fail(std::ostream& str) const;
    private:
        /// Stores whether compilation was successful.
        bool success;
        /// Stores an error message, if there is one.
        std::optional<std::string> error_message;
    };

    /**
     * Component of a shader program. This can be a vertex, tessellation-control, tessellation-evaluation, geometry, fragment or compute shader.
     */
    class OGLShaderComponent
    {
    public:
        /**
         * Construct a ShaderComponent from a given type and optionally, its source.
         * If its source is provided, it will be instantly uploaded and attempt compilation.
         * @param type - The type of this shader component, such as VERTEX shader
         * @param source - Source code of the shader, if it is provided now. It can be provided and compiled later if preferred
         */
        OGLShaderComponent(OGLShaderComponentType type, const std::optional<std::string>& source = std::nullopt);
        /**
         * Retrieve the type of this ShaderComponent.
         * @return - Type of the shader
         */
        const OGLShaderComponentType& get_type() const;
        /**
         * Upload the shader source code to VRAM. This alone does not compile the source code.
         * @param source - Source code of the shader
         */
        void upload_source(const std::string& source) const;
        /**
         * Compile the previously uploaded shader source code.
         * @return - CompileResult containing information about this compilation attempt
         */
        OGLShaderComponentCompileResult compile() const;
        /**
         * Retrieve the CompileResult for the most recent compilation attempt.
         * @return - Most recent compilation result
         */
        OGLShaderComponentCompileResult get_compile_result() const;

        friend class OGLShaderComponentCompileResult;
        friend class OGLShaderProgram;
    private:
        /// Underlying OpenGL handle.
        GLuint shader_handle;
        /// Type of this shader.
        OGLShaderComponentType type;
    };

    using UniformLocation = GLint;
    class OGLShaderProgramLinkResult;
    class OGLShaderProgramValidateResult;
    class UniformImplicit;
    template<typename T>
    class Uniform;

    /**
     * OpenGL GLSL shader program wrapper.
     */
    class OGLShaderProgram
    {
    public:
        /**
         * Declare a ShaderProgram.
         */
        OGLShaderProgram();
        /**
         * Query as to whether this program supports tessellation. This will return true if there is a component that is of type TESSELLATION_CONTROL.
         * @return - True if tessellation is supported, otherwise false
         */
        bool get_can_tessellate() const;
        /**
         * Retrieve a pointer to a ShaderComponent matching the given type, if there is one.
         * @param type - Type of the ShaderComponent to retrieve
         * @return - Pointer to the first (and only) ShaderComponent matching the given type. If no such component exists with the type, nullptr is returned
         */
        const OGLShaderComponent* get_shader_component(OGLShaderComponentType type) const;
        /**
         * Construct a ShaderComponent in-place and attach it to this program.
         * @tparam Args - Types of the paramaters used to construct the ShaderComponent
         * @param args - Values to construct the ShaderComponent
         * @return - Reference to the resultant ShaderComponent.
         */
        template<typename... Args>
        OGLShaderComponent& emplace_shader_component(Args&&... args);
        /**
         * Retrieve the location of the uniform with the given name, if it exists.
         * @param uniform_name - Name of the uniform
         * @return - Location of the uniform if it exists, otherwise null
         */
        std::optional<UniformLocation> get_uniform_location(const std::string& uniform_name) const;
        /**
         * Construct a generic Uniform in-place available to all attached ShaderComponents.
         * @tparam T - Underlying type of the uniform
         * @tparam Args - Argument types used to construct the uniform
         * @param args - Values used to construct the uniform
         * @return - Reference to the resultant Uniform<T>
         */
        template<typename T, typename... Args>
        Uniform<T>& emplace_uniform(Args&&... args);
        /**
         * Set the value of a specific strongly-typed Uniform to be equal to the given value.
         * If the uniform with the name doesn't exist, it is created with this value.
         * @tparam T - Underlying type of the uniform
         * @param uniform_location - Name of the uniform
         * @param value - Desired value of the uniform
         */
        template<typename T>
        void set_uniform(const std::string& uniform_location, T value);
        /**
         * Retrieve a pointer to the abstract class representing the uniform with the given name, if it exists (weakly-typed).
         * @param uniform_location - Name of the uniform to retrieve
         * @return - Pointer to the weakly-typed uniform if it exists, otherwise nullptr
         */
        const UniformImplicit* get_uniform(const std::string& uniform_location) const;
        /**
         * Retrieve the current value of the uniform with the given name.
         * @tparam T - Underlying type of the uniform
         * @param uniform_location - Name of the uniform
         * @return - Pointer to the value of the uniform if it exists, otherwise nullptr
         */
        template<typename T>
        const T* get_uniform_value(const std::string& uniform_location) const;
        /**
         * Query as to whether a uniform with the given name exists.
         * @param uniform_name - Name of the uniform
         * @return - True if the uniform exists, otherwise false
         */
        bool has_uniform(const std::string& uniform_name) const;
        /**
         * Bind the given index to an attribute with the given name.
         * Usually bind_attribute(...) should be preferred, but this would be required if you are implementing your own attribute functionality.
         * @param index - Index of the attribute to bind
         * @param name - Name of the attribute
         */
        void bind_attribute_location(GLuint index, const std::string& name) const;
        /**
         * Bind the given VertexAttribute with the given name.
         * @param attribute - VertexAttribute to bind to the name
         * @param name - Name of the attribute
         */
        void bind_attribute(const OGLVertexAttribute& attribute, const std::string& name) const;
        /**
         * Query as to whether all attached ShaderComponents have been compiled successfully.
         * @return - True if all ShaderComponents are properly compiled, otherwise false
         */
        bool is_fully_compiled() const;
        /**
         * Link the ShaderProgram. This requires the minimum ShaderComponents to be attached (at least a vertex and fragment shader).
         * If additional ShaderComponents are attached, then they too must have already been successfully compiled, otherwise the linkage shall fail and be indicated via the return value.
         * @return - LinkResult containing information about the link attempt
         */
        OGLShaderProgramLinkResult link() const;
        /**
         * Retrieve the LinkResult for the most recent link attempt.
         * @return - Most recent linkage result
         */
        OGLShaderProgramLinkResult get_link_result() const;
        /**
         * Validate the ShaderProgram, ensuring that it is ready for use. This requires that linkage has already occurred successfully with no additional ShaderComponents attached since.
         * @return - ValidateResult containing information about the validation attempt
         */
        OGLShaderProgramValidateResult validate() const;
        /**
         * Retrieve the ValidateResult for the most recent validation attempt.
         * @return - Most recent validation result
         */
        OGLShaderProgramValidateResult get_validate_result() const;
        /**
         * Specify a value to record in transform feedback.
         * @param output_name - Name of the varying variable to use for transform feedback
         * @param interleaved - Whether the mode of capturing the varying variables is interleaved or simply separate
         */
        void set_output_feedback(const std::string& output_name, bool interleaved) const;
        /**
         * Bind the shader, causing subsequent render-calls to utilise this.
         */
        void bind() const;
        /**
         * Update the shader, pushing all updated uniforms to VRAM.
         */
        void update();

        friend class OGLShaderProgramLinkResult;
        friend class OGLShaderProgramValidateResult;
    private:
        /// Underlying OpenGL program handle.
        GLuint program_handle;
        /// Container of all attached ShaderComponents.
        std::vector<std::unique_ptr<OGLShaderComponent>> components;
        /// Container of all attached polymorphic Uniforms.
        std::vector<std::unique_ptr<UniformImplicit>> uniforms;
    };

    /**
     * Contains information about the results of a previous link attempt.
     */
    class OGLShaderProgramLinkResult
    {
    public:
        /**
         * Construct a result based upon the most recent linkage of a given program.
         * @param shader_program - The shader which was previously attempted to be linked
         */
        OGLShaderProgramLinkResult(const OGLShaderProgram& shader_program);
        /**
         * Construct a result where no attempt to link was made. This shall be invoked by OGLShaderProgram::link() when not all components are successfully compiled, for example.
         */
        OGLShaderProgramLinkResult();
        /**
         * Query as to whether linkage was even attempted in the first place. This will return false if the default constructor was invoked
         * @return - True if linkage was attempted, otherwise false
         */
        bool get_was_attempted() const;
        /**
         * Query as to whether the linkage was successful.
         * @return - True if linkage was successful, otherwise false
         */
        bool was_successful() const;
        /**
         * Report the error message to the given std::ostream.
         * @param str - The stream via which to report the compilation error message
         */
        void report(std::ostream& str) const;
        /**
         * Invokes report(str) if the compilation result was a failure.
         * @param str - The stream via which to report the compilation error message
         */
        void report_if_fail(std::ostream& str) const;
    private:
        /// Stores if linkage was attempted.
        bool was_attempted;
        /// Stores if linkage was a success.
        bool success;
        /// Stores the error message, if there was one.
        std::optional<std::string> error_message;
    };

    class OGLShaderProgramValidateResult
    {
    public:
        /**
         * Construct a result based upon the most recent validation of a given program.
         * @param shader_program - The shader which was previously attempted to be validated
         */
        OGLShaderProgramValidateResult(const OGLShaderProgram& shader_program);
        /**
         * Construct a result where no attempt to validate was made. This shall be invoked by OGLShaderProgram::validate() when a successful linkage has not yet occurred, for example.
         */
        OGLShaderProgramValidateResult();
        /**
         * Query as to whether linkage was even attempted in the first place. This will return false if the default constructor was invoked
         * @return - True if linkage was attempted, otherwise false
         */
        bool get_was_attempted() const;
        /**
         * Query as to whether the linkage was successful.
         * @return - True if linkage was successful, otherwise false
         */
        bool was_successful() const;
        /**
         * Report the error message to the given std::ostream.
         * @param str - The stream via which to report the compilation error message
         */
        void report(std::ostream& str) const;
        /**
         * Invokes report(str) if the compilation result was a failure.
         * @param str - The stream via which to report the compilation error message
         */
        void report_if_fail(std::ostream& str) const;
    private:
        /// Stores if linkage was attempted.
        bool was_attempted;
        /// Stores if linkage was a success.
        bool success;
        /// Stores the error message, if there was one.
        std::optional<std::string> error_message;
    };
}
#endif
#include "platform_specific/shader_program.inl"

#endif //TOPAZ_SHADER_PROGRAM_HPP