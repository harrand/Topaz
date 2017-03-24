#include "shader.hpp"

Shader::Shader(std::string filename)
{
	this->filename = filename;
	
	// Allocate space on GPU memory for shader.
	this->programHandle = glCreateProgram();
	// Vertex Shader
	this->shaders[0] = Shader::createShader(Shader::loadShader(this->filename + ".vertex.glsl"), GL_VERTEX_SHADER);
	// Fragment Shader
	this->shaders[1] = Shader::createShader(Shader::loadShader(this->filename + ".fragment.glsl"), GL_FRAGMENT_SHADER);
	
	for(unsigned int i = 0; i < 2; i++)
		glAttachShader(this->programHandle, this->shaders[i]);
	
	glBindAttribLocation(this->programHandle, 0, "position");
	glBindAttribLocation(this->programHandle, 1, "texcoord");
	glBindAttribLocation(this->programHandle, 2, "normal");
	
	glLinkProgram(this->programHandle);
	Shader::checkShaderError(this->programHandle, GL_LINK_STATUS, true, "Fatal Error: Program Linking failed.");
	
	glValidateProgram(this->programHandle);
	Shader::checkShaderError(this->programHandle, GL_VALIDATE_STATUS, true, "Fatal Error: Program Validation failed.");
	
	this->uniforms[(unsigned int)UniformTypes::MODEL] = glGetUniformLocation(this->programHandle, "m");
	this->uniforms[(unsigned int)UniformTypes::VIEW] = glGetUniformLocation(this->programHandle, "v");
	this->uniforms[(unsigned int)UniformTypes::PROJECTION] = glGetUniformLocation(this->programHandle, "p");
	this->uniforms[(unsigned int)UniformTypes::DISPLACEMENT_MAP_SCALE] = glGetUniformLocation(this->programHandle, "displacementMultiplier");
	this->uniforms[(unsigned int)UniformTypes::DISPLACEMENT_MAP_BIAS] = glGetUniformLocation(this->programHandle, "displacementBias");
}

Shader::~Shader()
{
	for(unsigned int i = 0; i < 2; i++)
	{
		glDetachShader(this->programHandle, this->shaders[i]);
		glDeleteShader(this->shaders[i]);
	}
	// Free GPU memory
	glDeleteProgram(this->programHandle);
}

GLuint Shader::getProgramHandle() const
{
	return this->programHandle;
}

void Shader::bind() const
{
	glUseProgram(this->programHandle);
}

void Shader::update(float (&modeldata)[16], float (&viewdata)[16], float (&projectiondata)[16], float displacementMapScale, float displacementMapOffset) const
{
	glUniformMatrix4fv(this->uniforms[(unsigned int)UniformTypes::MODEL], 1, GL_TRUE, modeldata);
	glUniformMatrix4fv(this->uniforms[(unsigned int)UniformTypes::VIEW], 1, GL_TRUE, viewdata);
	glUniformMatrix4fv(this->uniforms[(unsigned int)UniformTypes::PROJECTION], 1, GL_TRUE, projectiondata);
	glUniform1f(this->uniforms[(unsigned int)UniformTypes::DISPLACEMENT_MAP_SCALE], displacementMapScale);
	float defaultBias = displacementMapScale / 2.0f;
	glUniform1f(this->uniforms[(unsigned int)UniformTypes::DISPLACEMENT_MAP_BIAS], -defaultBias + defaultBias * displacementMapOffset);
}

std::string Shader::loadShader(const std::string& filename)
{
    std::ifstream file;
    file.open((filename).c_str());

    std::string output;
    std::string line;

    if(file.is_open())
    {
        while(file.good())
        {
            getline(file, line);
			output.append(line + "\n");
        }
    }
    else
    {
		std::cerr << "Unable to load shader: " << filename << std::endl;
    }
    return output;
}

void Shader::checkShaderError(GLuint shader, GLuint flag, bool isProgram, std::string errorMessage)
{
    GLint success = 0;
    GLchar error[1024] = {0};

    if(isProgram)
        glGetProgramiv(shader, flag, &success);
    else
        glGetShaderiv(shader, flag, &success);

    if(success == GL_FALSE)
    {
        if(isProgram)
            glGetProgramInfoLog(shader, sizeof(error), NULL, error);
        else
            glGetShaderInfoLog(shader, sizeof(error), NULL, error);

        std::cerr << errorMessage << ": '" << error << "'" << std::endl;
    }
}

GLuint Shader::createShader(std::string source, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	if(shader == 0)
		std::cerr << "Fatal Error: Shader Creation failed (Perhaps out of memory?)\n";
	
	const GLchar* shaderSources[1];
	GLint shaderSourceLengths[1];
	
	shaderSources[0] = source.c_str();
	shaderSourceLengths[0] = source.length();
	
	glShaderSource(shader, 1, shaderSources, shaderSourceLengths);
	glCompileShader(shader);
	
	Shader::checkShaderError(shader, GL_COMPILE_STATUS, false, "Fatal Error: Shader Compilation failed:\n");
	
	return shader;
}
