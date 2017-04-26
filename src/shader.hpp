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
	Shader(const Shader& copy);
	Shader(Shader&& move);
	Shader& operator=(const Shader& rhs) = delete;
	~Shader();
	GLuint getProgramHandle() const;
	void bind() const;
	void update(const std::vector<float>&& modelData, const std::vector<float>&& viewData, const std::vector<float>&& projectionData, float parallaxMapScale = 0.04f, float parallaxMapOffset = -0.5f) const;
private:
	static std::string loadShader(const std::string& filename);
	static void checkShaderError(GLuint shader, GLuint flag, bool isProgram, std::string errorMessage);
	static GLuint createShader(std::string source, GLenum shaderType);
	enum class UniformTypes : unsigned int
	{
		MODEL = 0,
		VIEW = 1,
		PROJECTION = 2,
		PARALLAX_MAP_SCALE = 3,
		PARALLAX_MAP_BIAS = 4,
		NUM_UNIFORMS = 5
	};
	std::string filename;
	GLuint programHandle;
	GLuint shaders[3];
	GLuint uniforms[(unsigned int)UniformTypes::NUM_UNIFORMS];
};

#endif