#include "graphics/shader.hpp"
#include "utility/file.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
    OGLShader::OGLShader(std::string vertex_source, std::string tessellation_control_source, std::string tessellation_evaluation_source, std::string geometry_source, std::string fragment_source, bool compile, bool link, bool validate) : filename(""), compiled(false), program_handle(glCreateProgram()), uniform_data({nullptr}), uniform_counter(0)
    {
        if (compile)
            this->compile(vertex_source, tessellation_control_source, tessellation_evaluation_source, geometry_source, fragment_source);
        if (link)
            this->link();
        if (validate)
            this->validate();
    }

    OGLShader::OGLShader(std::string filename, bool compile, bool link, bool validate) : OGLShader(::tz::utility::file::read(filename + ".vertex.glsl"), ::tz::utility::file::read(filename + ".tessellation_control.glsl"), ::tz::utility::file::read(filename + ".tessellation_evaluation.glsl"), ::tz::utility::file::read(filename + ".geometry.glsl"), ::tz::utility::file::read(filename + ".fragment.glsl"), compile, link, validate)
    {
        // Delegating ctor means cannot initialise any members after, and doing before will just be overwritten so that's why it's being done in this constructor body.
        this->filename = filename;
    }

    OGLShader::OGLShader(const OGLShader &copy) : OGLShader(copy.filename) {}

    OGLShader::OGLShader(OGLShader &&move) : filename(std::move(move.filename)), program_handle(std::move(move.program_handle)), uniform_data(std::move(move.uniform_data)), attribute_locations(std::move(move.attribute_locations))
    {
        for (std::size_t i = 0; i < ::tz::graphics::maximum_shaders; i++)
        {
            this->shaders[i] = move.shaders[i];
            move.shaders[i] = 0;
        }
        move.uniform_data = {};
        move.attribute_locations = {};
        move.program_handle = 0;
        // Now when destructor of move is invoked, nothing is attempted to be deleted or detached so the shader lives on in this instance.
    }

    OGLShader::~OGLShader()
    {
        // If this was moved and this destructor was invoked, then the program_handle will be zero (cant normally be zero so we skip all of this crap to avoid crashes)
        if (this->program_handle == 0)
            return;
        for (std::size_t i = 0; i < ::tz::graphics::maximum_shaders; i++)
        {
            if (this->shaders[i] == 0)
                continue;
            glDetachShader(this->program_handle, this->shaders[i]);
            glDeleteShader(this->shaders[i]);
        }
        // Free GPU memory
        glDeleteProgram(this->program_handle);
    }

    void OGLShader::compile(std::string vertex_source, std::string tessellation_control_source, std::string tessellation_evaluation_source, std::string geometry_source, std::string fragment_source)
    {
        // Vertex OGLShader
        this->shaders[0] = OGLShader::create_shader(vertex_source, GL_VERTEX_SHADER);
        // Tessellation Control OGLShader
        this->shaders[1] = OGLShader::create_shader(tessellation_control_source, GL_TESS_CONTROL_SHADER);
        // Tessellation Evaluation OGLShader
        this->shaders[2] = OGLShader::create_shader(tessellation_evaluation_source, GL_TESS_EVALUATION_SHADER);
        // Geometry OGLShader
        this->shaders[3] = OGLShader::create_shader(geometry_source, GL_GEOMETRY_SHADER);
        // Fragment OGLShader
        this->shaders[4] = OGLShader::create_shader(fragment_source, GL_FRAGMENT_SHADER);
        for (std::size_t i = 0; i < ::tz::graphics::maximum_shaders; i++)
            if (this->shaders[i] != 0)
                glAttachShader(this->program_handle, this->shaders[i]);

        // bind the attributes needed normally
        // TODO: Improve this to be actually flexible.
        this->attribute_locations[0] = "position";
        this->attribute_locations[1] = "texcoord";
        this->attribute_locations[2] = "normal";
        this->attribute_locations[3] = "tangent";
        // extras for instancing purposes.
        this->attribute_locations[4] = "instancing_model_x";
        this->attribute_locations[5] = "instancing_model_y";
        this->attribute_locations[6] = "instancing_model_z";
        this->attribute_locations[7] = "instancing_model_w";
        for (auto[id, location] : this->attribute_locations)
        {
            glBindAttribLocation(this->program_handle, id, location.c_str());
        }
        this->compiled = true;
    }

    void OGLShader::link()
    {
        glLinkProgram(this->program_handle);
        OGLShader::check_shader_error(this->program_handle, GL_LINK_STATUS, true, "OGLShader Program Linking Failed:\n");
    }

    void OGLShader::validate()
    {
        glValidateProgram(this->program_handle);
        OGLShader::check_shader_error(this->program_handle, GL_VALIDATE_STATUS, true, "OGLShader Program Validation Failed:\n");
    }

    bool OGLShader::is_compiled() const
    {
        return this->compiled;
    }

    bool OGLShader::is_linked() const
    {
        GLint status;
        glGetProgramiv(this->program_handle, GL_LINK_STATUS, &status);
        return status == GL_TRUE;
    }

    bool OGLShader::is_validated() const
    {
        GLint status;
        glGetProgramiv(this->program_handle, GL_VALIDATE_STATUS, &status);
        return status == GL_TRUE;
    }

    bool OGLShader::ready() const
    {
        return this->is_compiled() && this->is_linked() && this->is_validated();
    }

    void OGLShader::remove_uniform(std::string_view uniform_location)
    {
        if (this->uniform_counter == 0)
        {
            ::tz::debug::print("OGLShader::remove_uniform(...): Error: Tried to remove uniform location '", uniform_location, "' from OGLShader with handle ", this->program_handle, ", which does not currently have any attached uniforms.\n");
            return;
        }
        for (std::size_t i = 0; i < this->uniform_counter; i++)
        {
            if (this->uniform_data[i]->get_uniform_location() == uniform_location)
            {
                this->uniform_data[i].reset(nullptr);
                this->uniform_counter--;
            }
        }
    }

    bool OGLShader::has_uniform(std::string_view uniform_location) const
    {
        for (std::size_t i = 0; i < this->uniform_counter; i++)
            if (this->uniform_data[i]->get_uniform_location() == uniform_location)
                return true;
        return false;
    }

    UniformImplicit *OGLShader::get_uniform(std::string_view uniform_location) const
    {
        for (std::size_t i = 0; i < this->uniform_counter; i++)
            if (this->uniform_data[i]->get_uniform_location() == uniform_location)
                return this->uniform_data[i].get();
        ::tz::debug::print("OGLShader::get_uniform(...): Failed to find uniform location '", uniform_location, "' in OGLShader with handle ", this->program_handle, ".\n");
        return nullptr;
    }

    std::size_t OGLShader::number_active_uniforms() const
    {
        return this->uniform_counter;
    }

    const std::string &OGLShader::get_attribute_location(std::size_t attribute_id) const
    {
        return this->attribute_locations.at(attribute_id);
    }

    void OGLShader::register_attribute(std::size_t attribute_id, std::string attribute_location)
    {
        this->attribute_locations[attribute_id] = attribute_location;
    }

    bool OGLShader::has_vertex_shader() const
    {
        return this->shaders[0] != 0;
    }

    bool OGLShader::has_tessellation_control_shader() const
    {
        return this->shaders[1] != 0;
    }

    bool OGLShader::has_tessellation_evaluation_shader() const
    {
        return this->shaders[2] != 0;
    }

    bool OGLShader::has_geometry_shader() const
    {
        return this->shaders[3] != 0;
    }

    bool OGLShader::has_fragment_shader() const
    {
        return this->shaders[4] != 0;
    }

    GLuint OGLShader::get_program_handle() const
    {
        return this->program_handle;
    }

    void OGLShader::bind() const
    {
        if (!this->ready())
        {
            ::tz::debug::print("OGLShader::bind(): Error: Attempted to bind OGLShader that is not ready.\n");
            return;
        }
        glUseProgram(this->program_handle);
    }

    void OGLShader::update() const
    {
        // literally just update uniforms with the parameters
        for (std::size_t i = 0; i < this->uniform_counter; i++)
            this->uniform_data[i]->push();
    }

    void OGLShader::check_shader_error(GLuint shader, GLuint flag, bool is_program, std::string error_message)
    {
        GLint success = 0;
        GLchar error[1024] = {0};

        if (is_program)
            glGetProgramiv(shader, flag, &success);
        else
            glGetShaderiv(shader, flag, &success);

        if (is_program)
            glGetProgramInfoLog(shader, sizeof(error), NULL, error);
        else
            glGetShaderInfoLog(shader, sizeof(error), NULL, error);
        if (success == GL_TRUE && std::string(error) != "")
            ::tz::debug::print("OGLShader::check_shader_error(...): Success, Log:\n", error, "\n");
        else if (success != GL_TRUE)
            ::tz::debug::print(error_message, error, "\n");
    }

    GLuint OGLShader::create_shader(std::string source, GLenum shader_type)
    {
        if (source == "")
        {
            ::tz::debug::print("OGLShader::create_shader(...): OGLShader Source for Type [", ::tz::util::shader_type_string(shader_type), "] was empty, skipping it.\n");
            return 0;
        }
        GLuint shader = glCreateShader(shader_type);
        if (shader == 0)
        {
            ::tz::debug::print("OGLShader::create_shader(...): Error: OGLShader Creation failed (Perhaps out of memory?).\n");
            return 0;
        }

        // opengl has support for multiple shader sources so must pass it an array. we're (meant to be) sane so we only need one source per shader actually thank you very much
        const GLchar *shader_sources[1];
        GLint shader_source_lengths[1];

        shader_sources[0] = source.c_str();
        shader_source_lengths[0] = source.length();

        glShaderSource(shader, 1, shader_sources, shader_source_lengths);
        glCompileShader(shader);

        OGLShader::check_shader_error(shader, GL_COMPILE_STATUS, false, "OGLShader Compilation Failed:\n");

        return shader;
    }

    OGLShader tz::graphics::shader::pass_through(std::string position_attribute_name, std::string texture_coordinate_attribute_name, std::string texture_sampler_name)
    {
        constexpr char vertex_source[] = "#version 430\n\
\n\
layout(location = 0) in vec3 %POSITION%;\n\
layout(location = 1) in vec2 %TEXTURE_COORDINATE%;\n\
out vec3 position_modelspace;\n\
out vec2 texture_coordinate_modelspace;\n\
\n\
uniform mat4 m;\n\
uniform mat4 v;\n\
uniform mat4 p;\n\
\n\
void main()\n\
{\n\
	gl_Position = (p * v * m) * vec4(%POSITION%, 1.0);\n\
	position_modelspace = %POSITION%;\n\
	texture_coordinate_modelspace = %TEXTURE_COORDINATE%;\n\
}";
        constexpr char fragment_source[] = "#version 430\n\
\n\
layout(location = 0) out vec4 frag_colour;\n\
\n\
in vec2 texture_coordinate_modelspace;\n\
uniform sampler2D %TEXTURE_SAMPLER%;\n\
\n\
void main()\n\
{\n\
	frag_colour = texture(%TEXTURE_SAMPLER%, texture_coordinate_modelspace);\n\
}";
        using namespace ::tz::utility; // tz::utility::string
        return {string::replace_all(string::replace_all(vertex_source, "%POSITION%", position_attribute_name), "%TEXTURE_COORDINATE%", texture_coordinate_attribute_name), "", "", "", string::replace_all(fragment_source, "%TEXTURE_SAMPLER%", texture_sampler_name)};
    }
}
#endif

const char* tz::util::shader_type_string(GLenum shader_type)
{
    const char *shader_type_str;
    switch (shader_type)
    {
        case GL_COMPUTE_SHADER:
            shader_type_str = "Compute";
            break;
        case GL_VERTEX_SHADER:
            shader_type_str = "Vertex";
            break;
        case GL_TESS_CONTROL_SHADER:
            shader_type_str = "Tessellation Control";
            break;
        case GL_TESS_EVALUATION_SHADER:
            shader_type_str = "Tessellation Evaluation";
            break;
        case GL_GEOMETRY_SHADER:
            shader_type_str = "Geometry";
            break;
        case GL_FRAGMENT_SHADER:
            shader_type_str = "Fragment";
            break;
        default:
            shader_type_str = "Unknown";
            break;
    }
    return shader_type_str;
}