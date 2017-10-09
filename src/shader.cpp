#include "shader.hpp"
#include "matrix.hpp"
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
	if(this->ready())
		this->initialiseUniforms();
	tz::util::log::message("Shader with link '", this->filename, "':");
	tz::util::log::message("\tHas Vertex Shader: ", this->hasVertexShader());
	tz::util::log::message("\tHas Tessellation Control Shader: ", this->hasTessellationControlShader());
	tz::util::log::message("\tHas Tessellation Evaluation Shader: ", this->hasTessellationEvaluationShader());
	tz::util::log::message("\tHas Geometry Shader: ", this->hasGeometryShader());
	tz::util::log::message("\tHas Fragment Shader: ", this->hasFragmentShader());
}

Shader::Shader(const Shader& copy): Shader(copy.filename){}

Shader::Shader(Shader&& move): filename(move.filename), program_handle(move.program_handle)
{
	for(std::size_t i = 0; i < tz::graphics::maximum_shaders; i++)
	{
		this->shaders[i] = move.shaders[i];
		move.shaders[i] = 0;
	}
	for(std::size_t i = 0; i < static_cast<std::size_t>(UniformTypes::NUM_UNIFORMS); i++)
	{
		this->uniforms[i] = move.uniforms[i];
		move.uniforms[i] = 0;
	}
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
	this->shaders[0] = Shader::createShader(Shader::loadShader(this->filename + ".vertex.glsl"), GL_VERTEX_SHADER);
	// Tessellation Control Shader
	this->shaders[1] = Shader::createShader(Shader::loadShader(this->filename + ".tessellation_control.glsl"), GL_TESS_CONTROL_SHADER);
	// Tessellation Evalution Shader
	this->shaders[2] = Shader::createShader(Shader::loadShader(this->filename + ".tessellation_evaluation.glsl"), GL_TESS_EVALUATION_SHADER);
	// Geometry Shader
	this->shaders[3] = Shader::createShader(Shader::loadShader(this->filename + ".geometry.glsl"), GL_GEOMETRY_SHADER);
	// Fragment Shader
	this->shaders[4] = Shader::createShader(Shader::loadShader(this->filename + ".fragment.glsl"), GL_FRAGMENT_SHADER);
	for(std::size_t i = 0; i < tz::graphics::maximum_shaders; i++)
		if(this->shaders[i] != 0)
			glAttachShader(this->program_handle, this->shaders[i]);
	
	// bind the attributes needed normally
	glBindAttribLocation(this->program_handle, 0, "position");
	glBindAttribLocation(this->program_handle, 1, "texcoord");
	glBindAttribLocation(this->program_handle, 2, "normal");
	glBindAttribLocation(this->program_handle, 3, "tangent");
	this->compiled = true;
}

void Shader::link()
{
	glLinkProgram(this->program_handle);
	Shader::checkShaderError(this->program_handle, GL_LINK_STATUS, true, "Shader Program Linking Failed:\n");
}

void Shader::validate()
{
	glValidateProgram(this->program_handle);
	Shader::checkShaderError(this->program_handle, GL_VALIDATE_STATUS, true, "Shader Program Validation Failed:\n");
}

bool Shader::isCompiled() const
{
	return this->compiled;
}

bool Shader::isLinked() const
{
	GLint status;
	glGetProgramiv(this->program_handle, GL_LINK_STATUS, &status);
	return status == GL_TRUE;
}

bool Shader::isValidated() const
{
	GLint status;
	glGetProgramiv(this->program_handle, GL_VALIDATE_STATUS, &status);
	return status == GL_TRUE;
}

bool Shader::ready() const
{
	return this->isCompiled() && this->isLinked() && this->isValidated();
}

void Shader::initialiseUniforms()
{
	this->uniforms[static_cast<std::size_t>(UniformTypes::MODEL)] = glGetUniformLocation(this->program_handle, "m");
	this->uniforms[static_cast<std::size_t>(UniformTypes::VIEW)] = glGetUniformLocation(this->program_handle, "v");
	this->uniforms[static_cast<std::size_t>(UniformTypes::PROJECTION)] = glGetUniformLocation(this->program_handle, "p");
	this->uniforms[static_cast<std::size_t>(UniformTypes::SHININESS)] = glGetUniformLocation(this->program_handle, "shininess");
	this->uniforms[static_cast<std::size_t>(UniformTypes::PARALLAX_MAP_SCALE)] = glGetUniformLocation(this->program_handle, "parallax_multiplier");
	this->uniforms[static_cast<std::size_t>(UniformTypes::PARALLAX_MAP_BIAS)] = glGetUniformLocation(this->program_handle, "parallax_bias");
	this->uniforms[static_cast<std::size_t>(UniformTypes::DISPLACEMENT_FACTOR)] = glGetUniformLocation(this->program_handle, "displacement_factor");
}

bool Shader::hasVertexShader() const
{
	return this->shaders[0] != 0;
}

bool Shader::hasTessellationControlShader() const
{
	return this->shaders[1] != 0;
}

bool Shader::hasTessellationEvaluationShader() const
{
	return this->shaders[2] != 0;
}

bool Shader::hasGeometryShader() const
{
	return this->shaders[3] != 0;
}

bool Shader::hasFragmentShader() const
{
	return this->shaders[4] != 0;
}

GLuint Shader::getProgramHandle() const
{
	return this->program_handle;
}

void Shader::bind() const
{
	glUseProgram(this->program_handle);
}

void Shader::update(const std::array<float, 16>& model_matrix_array, const std::array<float, 16>& view_matrix_array, const std::array<float, 16>& projection_matrix_array, unsigned int shininess, float parallaxmap_scale_constant, float parallaxmap_offset_constant, float displacement_factor) const
{
	// literally just update uniforms with the parameters
	glUniformMatrix4fv(this->uniforms[static_cast<unsigned int>(UniformTypes::MODEL)], 1, GL_TRUE, model_matrix_array.data());
	glUniformMatrix4fv(this->uniforms[static_cast<unsigned int>(UniformTypes::VIEW)], 1, GL_TRUE, view_matrix_array.data());
	glUniformMatrix4fv(this->uniforms[static_cast<unsigned int>(UniformTypes::PROJECTION)], 1, GL_TRUE, projection_matrix_array.data());
	glUniform1ui(this->uniforms[static_cast<unsigned int>(UniformTypes::SHININESS)], shininess);
	glUniform1f(this->uniforms[static_cast<unsigned int>(UniformTypes::PARALLAX_MAP_SCALE)], parallaxmap_scale_constant);
	glUniform1f(this->uniforms[static_cast<unsigned int>(UniformTypes::PARALLAX_MAP_BIAS)], parallaxmap_scale_constant / 2.0f * (parallaxmap_offset_constant - 1));
	glUniform1f(this->uniforms[static_cast<unsigned int>(UniformTypes::DISPLACEMENT_FACTOR)], displacement_factor);
}

std::string Shader::loadShader(const std::string& filename)
{
	// use MDL rawfile to slurp the entire file and then parse each lines with a newline between each. might be an optimisation to use RawFile slurp function
	std::string source = "";
	for(std::string str : RawFile(filename).getLines())
		source += str + "\n";
	return source;
}

void Shader::checkShaderError(GLuint shader, GLuint flag, bool is_program, std::string error_message)
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

GLuint Shader::createShader(std::string source, GLenum shader_type)
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
	const GLchar* shaderSources[1];
	GLint shaderSourceLengths[1];
	
	shaderSources[0] = source.c_str();
	shaderSourceLengths[0] = source.length();
	
	glShaderSource(shader, 1, shaderSources, shaderSourceLengths);
	glCompileShader(shader);
	
	Shader::checkShaderError(shader, GL_COMPILE_STATUS, false, "Shader Compilation Failed:\n");
	
	return shader;
}
