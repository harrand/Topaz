#ifndef SHADER_HPP
#define SHADER_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#include <string>
#include <fstream>
#include <iostream>
#include "utility.hpp"
#include "glew.h"
#include "matrix.hpp"

class Shader
{
public:
	Shader(std::string filename);
	~Shader();
	GLuint getProgramHandle() const;
	void bind() const;
	void update(float (&modeldata)[16], float (&viewdata)[16], float (&projectiondata)[16], float displacementMapScale = 0.04f, float displacementMapOffset = -0.5f) const;
private:
	static std::string loadShader(std::string filename);
	static void checkShaderError(GLuint shader, GLuint flag, bool isProgram, std::string errorMessage);
	static GLuint createShader(std::string source, GLenum shaderType);
	
	enum class UniformTypes : unsigned int
	{
		MODEL = 0,
		VIEW = 1,
		PROJECTION = 2,
		DISPLACEMENT_MAP_SCALE = 3,
		DISPLACEMENT_MAP_BIAS = 4,
		NUM_UNIFORMS = 5
	};
	
	std::string filename;
	GLuint programHandle;
	GLuint shaders[2];
	GLuint uniforms[(unsigned int)UniformTypes::NUM_UNIFORMS];
};

#endif