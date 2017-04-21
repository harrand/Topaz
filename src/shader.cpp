#include "shader.hpp"

//const unsigned int Shader::MAXIMUM_LIGHTS = 8;

Shader::Shader(std::string filename): filename(filename)
{
	// Allocate space on GPU memory for shader.
	this->programHandle = glCreateProgram();
	// Vertex Shader
	this->shaders[0] = Shader::createShader(Shader::loadShader(this->filename + ".vertex.glsl"), GL_VERTEX_SHADER);
	// Fragment Shader
	this->shaders[1] = Shader::createShader(Shader::loadShader(this->filename + ".fragment.glsl"), GL_FRAGMENT_SHADER);
	// Geometry Shader
	this->shaders[2] = Shader::createShader(Shader::loadShader(this->filename + ".geometry.glsl"), GL_GEOMETRY_SHADER);
	
	for(unsigned int i = 0; i < 3; i++)
		glAttachShader(this->programHandle, this->shaders[i]);
	
	glBindAttribLocation(this->programHandle, 0, "position");
	glBindAttribLocation(this->programHandle, 1, "texcoord");
	glBindAttribLocation(this->programHandle, 2, "normal");
	glBindAttribLocation(this->programHandle, 3, "tangent");
	
	glLinkProgram(this->programHandle);
	Shader::checkShaderError(this->programHandle, GL_LINK_STATUS, true, "Program Linking failed");
	
	glValidateProgram(this->programHandle);
	Shader::checkShaderError(this->programHandle, GL_VALIDATE_STATUS, true, "Program Validation failed");
	
	this->uniforms[(unsigned int)UniformTypes::MODEL] = glGetUniformLocation(this->programHandle, "m");
	this->uniforms[(unsigned int)UniformTypes::VIEW] = glGetUniformLocation(this->programHandle, "v");
	this->uniforms[(unsigned int)UniformTypes::PROJECTION] = glGetUniformLocation(this->programHandle, "p");
	this->uniforms[(unsigned int)UniformTypes::PARALLAX_MAP_SCALE] = glGetUniformLocation(this->programHandle, "parallaxMultiplier");
	this->uniforms[(unsigned int)UniformTypes::PARALLAX_MAP_BIAS] = glGetUniformLocation(this->programHandle, "parallaxBias");
}

Shader::Shader(const Shader& copy): Shader(copy.filename){}

Shader::Shader(Shader&& move): filename(move.filename), programHandle(move.programHandle)
{
	for(unsigned int i = 0; i < 3; i++)
	{
		this->shaders[i] = move.shaders[i];
		move.shaders[i] = 0;
	}
	for(unsigned int i = 0; i < (unsigned int)UniformTypes::NUM_UNIFORMS; i++)
	{
		this->uniforms[i] = move.uniforms[i];
		move.uniforms[i] = 0;
	}
	move.programHandle = 0;
	// Now when destructor of move is invoked, nothing is attempted to be deleted or detached so the shader lives on in this instance.
}

Shader::~Shader()
{
	// If this was moved and this destructor was invoked, then the programHandle will be zero (cant normally be zero so we skip all of this crap to avoid crashes)
	if(this->programHandle == 0)
		return;
	for(unsigned int i = 0; i < 3; i++)
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
/*
const std::map<std::pair<GLuint, GLuint>, std::unique_ptr<BaseLight>>& Shader::getLights() const
{
	return this->baseLights;
}
*/

/*
void Shader::addLight(BaseLight&& light)
{
	while(this->baseLights.size() >= Shader::MAXIMUM_LIGHTS)
		this->baseLights.erase(this->baseLights.begin());
	//lights guaranteed to have at least one empty space now
	this->baseLights[std::make_pair<GLuint, GLuint>(glGetUniformLocation(this->programHandle, ("lights[" + CastUtility::toString<unsigned int>(this->baseLights.size()) + "].pos").c_str()), glGetUniformLocation(this->programHandle, ("lights[" + CastUtility::toString<unsigned int>(this->baseLights.size()) + "].power").c_str()))] = std::make_unique<BaseLight>(std::move(light));
}
*/

void Shader::bind() const
{
	glUseProgram(this->programHandle);
}

void Shader::update(const std::vector<float>&& modelData, const std::vector<float>&& viewData, const std::vector<float>&& projectionData, float parallaxMapScale, float parallaxMapOffset) const
{
	glUniformMatrix4fv(this->uniforms[(unsigned int)UniformTypes::MODEL], 1, GL_TRUE, &modelData[0]);
	glUniformMatrix4fv(this->uniforms[(unsigned int)UniformTypes::VIEW], 1, GL_TRUE, &viewData[0]);
	glUniformMatrix4fv(this->uniforms[(unsigned int)UniformTypes::PROJECTION], 1, GL_TRUE, &projectionData[0]);
	glUniform1f(this->uniforms[(unsigned int)UniformTypes::PARALLAX_MAP_SCALE], parallaxMapScale);
	glUniform1f(this->uniforms[(unsigned int)UniformTypes::PARALLAX_MAP_BIAS], parallaxMapScale / 2.0f * (parallaxMapOffset - 1));
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
		LogUtility::error("Unable to load shader '" + filename + "'\n");
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

        LogUtility::error(errorMessage + std::string(error));
    }
}

GLuint Shader::createShader(std::string source, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	if(shader == 0)
		LogUtility::error("Fatal Error: Shader Creation failed (Perhaps out of memory?)");
	
	const GLchar* shaderSources[1];
	GLint shaderSourceLengths[1];
	
	shaderSources[0] = source.c_str();
	shaderSourceLengths[0] = source.length();
	
	glShaderSource(shader, 1, shaderSources, shaderSourceLengths);
	glCompileShader(shader);
	
	Shader::checkShaderError(shader, GL_COMPILE_STATUS, false, "Fatal Error: Shader Compilation failed:\n");
	
	return shader;
}
