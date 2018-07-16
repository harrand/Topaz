#include "graphics/shader.hpp"
#include "utility/file.hpp"

UniformImplicit::UniformImplicit(GLuint shader_handle, std::string uniform_location): shader_handle(shader_handle), uniform_location(uniform_location){}

GLuint UniformImplicit::get_shader_handle() const
{
	return this->shader_handle;
}

std::string_view UniformImplicit::get_uniform_location() const
{
	return {this->uniform_location};
}

Uniform<DirectionalLight>::Uniform(GLuint shader_handle, std::string uniform_location, DirectionalLight value): UniformImplicit(shader_handle, uniform_location), value(value), direction_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".direction").c_str())), colour_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".colour").c_str())), power_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".power").c_str())){}

const DirectionalLight& Uniform<DirectionalLight>::get_value() const
{
	return this->value;
}

void Uniform<DirectionalLight>::set_value(DirectionalLight value)
{
	this->value = value;
}

void Uniform<DirectionalLight>::push() const
{
	glUniform3f(this->direction_uniform_handle, this->value.get_direction().x, this->value.get_direction().y, this->value.get_direction().z);
	glUniform3f(this->colour_uniform_handle, this->value.get_colour().x, this->value.get_colour().y, this->value.get_colour().z);
	glUniform1f(this->power_uniform_handle, this->value.get_power());
}

Uniform<PointLight>::Uniform(GLuint shader_handle, std::string uniform_location, PointLight value): UniformImplicit(shader_handle, uniform_location), value(value), position_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".position").c_str())), colour_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".colour").c_str())), power_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".power").c_str())){}

const PointLight& Uniform<PointLight>::get_value() const
{
	return this->value;
}

void Uniform<PointLight>::set_value(PointLight value)
{
	this->value = value;
}

void Uniform<PointLight>::push() const
{
	glUniform3f(this->position_uniform_handle, this->value.position.x, this->value.position.y, this->value.position.z);
	glUniform3f(this->colour_uniform_handle, this->value.get_colour().x, this->value.get_colour().y, this->value.get_colour().z);
	glUniform1f(this->power_uniform_handle, this->value.get_power());
}

Shader::Shader(std::string vertex_source, std::string tessellation_control_source, std::string tessellation_evaluation_source, std::string geometry_source, std::string fragment_source, bool compile, bool link, bool validate): filename(""), compiled(false), program_handle(glCreateProgram()), uniform_data({nullptr}), uniform_counter(0)
{
	if(compile)
		this->compile(vertex_source, tessellation_control_source, tessellation_evaluation_source, geometry_source, fragment_source);
	if(link)
		this->link();
	if(validate)
		this->validate();
}

Shader::Shader(std::string filename, bool compile, bool link, bool validate): Shader(tz::utility::file::read(filename + ".vertex.glsl"), tz::utility::file::read(filename + ".tessellation_control.glsl"), tz::utility::file::read(filename + ".tessellation_evaluation.glsl"), tz::utility::file::read(filename + ".geometry.glsl"), tz::utility::file::read(filename + ".fragment.glsl"), compile, link, validate)
{
	// Delegating ctor means cannot initialise any members after, and doing before will just be overwritten so that's why it's being done in this constructor body.
	this->filename = filename;
}

Shader::Shader(const Shader& copy): Shader(copy.filename){}

Shader::Shader(Shader&& move): filename(std::move(move.filename)), program_handle(std::move(move.program_handle)), uniform_data(std::move(move.uniform_data)), attribute_locations(std::move(move.attribute_locations))
{
	for(std::size_t i = 0; i < tz::graphics::maximum_shaders; i++)
	{
		this->shaders[i] = move.shaders[i];
		move.shaders[i] = 0;
	}
	move.uniform_data = {};
	move.attribute_locations = {};
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

void Shader::compile(std::string vertex_source, std::string tessellation_control_source, std::string tessellation_evaluation_source, std::string geometry_source, std::string fragment_source)
{
	// Vertex Shader
	this->shaders[0] = Shader::create_shader(vertex_source, GL_VERTEX_SHADER);
	// Tessellation Control Shader
	this->shaders[1] = Shader::create_shader(tessellation_control_source, GL_TESS_CONTROL_SHADER);
	// Tessellation Evaluation Shader
	this->shaders[2] = Shader::create_shader(tessellation_evaluation_source, GL_TESS_EVALUATION_SHADER);
	// Geometry Shader
	this->shaders[3] = Shader::create_shader(geometry_source, GL_GEOMETRY_SHADER);
	// Fragment Shader
	this->shaders[4] = Shader::create_shader(fragment_source, GL_FRAGMENT_SHADER);
	for(std::size_t i = 0; i < tz::graphics::maximum_shaders; i++)
		if(this->shaders[i] != 0)
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
	for(auto [id, location] : this->attribute_locations)
	{
		glBindAttribLocation(this->program_handle, id, location.c_str());
	}
	/*
	glBindAttribLocation(this->program_handle, 0, "position");
	glBindAttribLocation(this->program_handle, 1, "texcoord");
	glBindAttribLocation(this->program_handle, 2, "normal");
	glBindAttribLocation(this->program_handle, 3, "tangent");
	glBindAttribLocation(this->program_handle, 4, "positions_instance");
	glBindAttribLocation(this->program_handle, 5, "rotations_instance");
	glBindAttribLocation(this->program_handle, 6, "scales_instance");
	*/
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
	if(this->uniform_counter == 0)
	{
		std::cerr << "[Shader]: Tried to remove uniform location '" << uniform_location << "' from Shader with handle " << this->program_handle << ", which does not currently have any attached uniforms.\n";
		return;
	}
	for(std::size_t i = 0; i < this->uniform_counter; i++)
	{
		if(this->uniform_data[i]->get_uniform_location() == uniform_location)
		{
			this->uniform_data[i].reset(nullptr);
			this->uniform_counter--;
		}
	}
}

bool Shader::has_uniform(std::string_view uniform_location) const
{
	for(std::size_t i = 0; i < this->uniform_counter; i++)
		if(this->uniform_data[i]->get_uniform_location() == uniform_location)
			return true;
	return false;
}

UniformImplicit* Shader::get_uniform(std::string_view uniform_location) const
{
	for(std::size_t i = 0; i < this->uniform_counter; i++)
		if(this->uniform_data[i]->get_uniform_location() == uniform_location)
			return this->uniform_data[i].get();
	std::cerr << "[Shader]: Failed to find uniform location '" << uniform_location << "' in Shader with handle " << this->program_handle << ".\n";
	return nullptr;
}

std::size_t Shader::number_active_uniforms() const
{
	return this->uniform_counter;
}

const std::string& Shader::get_attribute_location(std::size_t attribute_id) const
{
	return this->attribute_locations.at(attribute_id);
}

void Shader::register_attribute(std::size_t attribute_id, std::string attribute_location)
{
	this->attribute_locations[attribute_id] = attribute_location;
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
    if(!this->ready())
    {
        std::cerr << "Attempted to bind Shader that is not ready.\n";
        return;
    }
	glUseProgram(this->program_handle);
}

void Shader::update() const
{
	// literally just update uniforms with the parameters
	for(std::size_t i = 0; i < this->uniform_counter; i++)
		this->uniform_data[i]->push();
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
		std::cout << "Success, Log:\n" << std::string(error) << "\n";
	else if(success != GL_TRUE)
		std::cerr << error_message << std::string(error) << "\n";
}

GLuint Shader::create_shader(std::string source, GLenum shader_type)
{
	if(source == "")
	{
		std::cout << "Shader Source for Type [" << tz::util::shader_type_string(shader_type) << "] was empty, skipping it.\n";
		return 0;
	}
	GLuint shader = glCreateShader(shader_type);
	if(shader == 0)
	{
		std::cerr << "Fatal Error: Shader Creation failed (Perhaps out of memory?).\n";
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

const char* tz::util::shader_type_string(GLenum shader_type)
{
	const char* shader_type_str;
	switch(shader_type)
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

Shader tz::graphics::shader::pass_through(std::string position_attribute_name, std::string texture_coordinate_attribute_name, std::string texture_sampler_name)
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
	using namespace tz::utility; // tz::utility::string
	return {string::replace_all(string::replace_all(vertex_source, "%POSITION%", position_attribute_name), "%TEXTURE_COORDINATE%", texture_coordinate_attribute_name), "", "", "", string::replace_all(fragment_source, "%TEXTURE_SAMPLER%", texture_sampler_name)};
}