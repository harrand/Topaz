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
	void update(float (&modeldata)[16], float (&viewdata)[16], float (&projectiondata)[16]) const;
private:
	static std::string loadShader(std::string filename);
	static void checkShaderError(GLuint shader, GLuint flag, bool isProgram, std::string errorMessage);
	static GLuint createShader(std::string source, GLenum shaderType);
	
	enum
	{
		MODEL_U,
		VIEW_U,
		PROJECTION_U,
		NUM_UNIFORMS
	};
	
	std::string filename;
	GLuint programHandle;
	GLuint shaders[2];
	GLuint uniforms[NUM_UNIFORMS];
};

#endif