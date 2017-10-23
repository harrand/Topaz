#include "shader.hpp"
#include <fstream>

Shader::Shader(std::string filename, bool compile, bool link, bool validate): filename(std::move(filename)), compiled(false)
{
	// Allocate space on GPU memory for shader.
	this->program_handle = glCreateProgram();
	if(compile)
		this->compile();
	if(link)
		this->link();
	if(validate)
		this->validate();
	tz::util::log::message("Shader with link '", this->filename, "':");
	tz::util::log::message("\t_has Vertex Shader: ", this->has_vertex_shader());
	tz::util::log::message("\t_has Tessellation Control Shader: ", this->has_tessellation_control_shader());
	tz::util::log::message("\t_has Tessellation Evaluation Shader: ", this->has_tessellation_evaluation_shader());
	tz::util::log::message("\t_has Geometry Shader: ", this->has_geometry_shader());
	tz::util::log::message("\t_has Fragment Shader: ", this->has_fragment_shader());
}

Shader::Shader(const Shader& copy): Shader(copy.filename){}

Shader::Shader(Shader&& move): filename(move.filename), program_handle(move.program_handle), uniform_data(move.uniform_data)
{
	for(std::size_t i = 0; i < tz::graphics::maximum_shaders; i++)
	{
		this->shaders[i] = move.shaders[i];
		move.shaders[i] = 0;
	}
	move.uniform_data = {};
	move.program_handle = 0;
	// Now when destructor of move is invoked, nothing is attempted to be deleted or detached so the shader lives on in this instance.
}

Shader::~Shader()
{
	// If this was moved and this destructor was invoked, then the program_handle will be zero (cant normally be zero so we skip all of this crap to avoid crashes)
	if(this->program_handle == 0)
		return;
	for(std::size_t i = 0; i < tz::graphics::maximum_shaders; i++)
	{
		if(this->shaders[i] == 0)
			continue;
		glDetachShader(this->program_handle, this->shaders[i]);
		glDeleteShader(this->shaders[i]);
	}
	// Free GPU memory
	glDeleteProgram(this->program_handle);
}

void Shader::compile()
{
	// Vertex Shader
	this->shaders[0] = Shader::create_shader(mdl::read(this->filename + ".vertex.glsl"), GL_VERTEX_SHADER);
	// Tessellation Control Shader
	this->shaders[1] = Shader::create_shader(mdl::read(this->filename + ".tessellation_control.glsl"), GL_TESS_CONTROL_SHADER);
	// Tessellation Evalution Shader
	this->shaders[2] = Shader::create_shader(mdl::read(this->filename + ".tessellation_evaluation.glsl"), GL_TESS_EVALUATION_SHADER);
	// Geometry Shader
	this->shaders[3] = Shader::create_shader(mdl::read(this->filename + ".geometry.glsl"), GL_GEOMETRY_SHADER);
	// Fragment Shader
	this->shaders[4] = Shader::create_shader(mdl::read(this->filename + ".fragment.glsl"), GL_FRAGMENT_SHADER);
	for(std::size_t i = 0; i < tz::graphics::maximum_shaders; i++)
		if(this->shaders[i] != 0)
			glAttachShader(this->program_handle, this->shaders[i]);
	
	// bind the attributes needed normally
	glBindAttribLocation(this->program_handle, 0, "position");
	glBindAttribLocation(this->program_handle, 1, "texcoord");
	glBindAttribLocation(this->program_handle, 2, "normal");
	glBindAttribLocation(this->program_handle, 3, "tangent");
	glBindAttribLocation(this->program_handle, 4, "positions_instance");
	glBindAttribLocation(this->program_handle, 5, "rotations_instance");
	glBindAttribLocation(this->program_handle, 6, "scales_instance");
	this->compiled = true;
}

void Shader::link()
{
	glLinkProgram(this->program_handle);
	Shader::check_shader_error(this->program_handle, GL_LINK_STATUS, true, "Shader Program Linking Failed:\n");
}

void Shader::validate()
{
	glValidateProgram(this->program_handle);
	Shader::check_shader_error(this->program_handle, GL_VALIDATE_STATUS, true, "Shader Program Validation Failed:\n");
}

bool Shader::is_compiled() const
{
	return this->compiled;
}

bool Shader::is_linked() const
{
	GLint status;
	glGetProgramiv(this->program_handle, GL_LINK_STATUS, &status);
	return status == GL_TRUE;
}

bool Shader::is_validated() const
{
	GLint status;
	glGetProgramiv(this->program_handle, GL_VALIDATE_STATUS, &status);
	return status == GL_TRUE;
}

bool Shader::ready() const
{
	return this->is_compiled() && this->is_linked() && this->is_validated();
}

void Shader::remove_uniform(std::string_view uniform_location)
{
	for(const auto& uniform : this->uniform_data)
		if(uniform->get_uniform_location() == uniform_location)
			this->uniform_data.erase(uniform);
}

bool Shader::has_uniform(std::string_view uniform_location) const
{
	for(const auto& uniform : this->uniform_data)
		if(uniform->get_uniform_location() == uniform_location)
			return true;
	return false;
}

UniformImplicit* Shader::get_uniform(std::string_view uniform_location) const
{
	for(const auto& uniform : this->uniform_data)
		if(uniform->get_uniform_location() == uniform_location)
			return uniform.get();
	return nullptr;
}

bool Shader::has_vertex_shader() const
{
	return this->shaders[0] != 0;
}

bool Shader::has_tessellation_control_shader() const
{
	return this->shaders[1] != 0;
}

bool Shader::has_tessellation_evaluation_shader() const
{
	return this->shaders[2] != 0;
}

bool Shader::has_geometry_shader() const
{
	return this->shaders[3] != 0;
}

bool Shader::has_fragment_shader() const
{
	return this->shaders[4] != 0;
}

GLuint Shader::get_program_handle() const
{
	return this->program_handle;
}

void Shader::bind() const
{
	glUseProgram(this->program_handle);
}

void Shader::update() const
{
	// literally just update uniforms with the parameters
	for(const auto& uniform : this->uniform_data)
		uniform->push();
}

void Shader::check_shader_error(GLuint shader, GLuint flag, bool is_program, std::string error_message)
{
    GLint success = 0;
    GLchar error[1024] = {0};

    if(is_program)
        glGetProgramiv(shader, flag, &success);
    else
        glGetShaderiv(shader, flag, &success);

	if(is_program)
		glGetProgramInfoLog(shader, sizeof(error), NULL, error);
	else
		glGetShaderInfoLog(shader, sizeof(error), NULL, error);
	if(success == GL_TRUE && std::string(error) != "")
		tz::util::log::message("Success, Log:\n", std::string(error));
	else if(success != GL_TRUE)
		tz::util::log::error(error_message, std::string(error));
}

GLuint Shader::create_shader(std::string source, GLenum shader_type)
{
	if(source == "")
	{
		tz::util::log::message("Shader Source for Type [", shader_type, "] was empty, skipping it.");
		return 0;
	}
	GLuint shader = glCreateShader(shader_type);
	if(shader == 0)
	{
		tz::util::log::error("Fatal Error: Shader Creation failed (Perhaps out of memory?)");
		return 0;
	}
	
	// opengl has support for multiple shader sources so must pass it an array. we're (meant to be) sane so we only need one source per shader actually thank you very much
	const GLchar* shader_sources[1];
	GLint shader_source_lengths[1];
	
	shader_sources[0] = source.c_str();
	shader_source_lengths[0] = source.length();
	
	glShaderSource(shader, 1, shader_sources, shader_source_lengths);
	glCompileShader(shader);
	
	Shader::check_shader_error(shader, GL_COMPILE_STATUS, false, "Shader Compilation Failed:\n");
	
	return shader;
}
