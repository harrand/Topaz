#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include <cassert>
#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "glew.h"
#include "utility.hpp"

class FrameBuffer
{
public:
	FrameBuffer(unsigned int width = 256, unsigned int height = 256);
	virtual void setRenderTarget() const;
	virtual void bind(unsigned int id) const;
private:
	unsigned int width, height;
	GLuint fbHandle, texHandle, depthRenderBufferHandle;
};

class Texture
{
public:
	Texture(std::string filename = "../../../res/runtime/textures/undefined.jpg");
	Texture(const Texture& copy);
	Texture(Texture&& rmove);
	~Texture();
	void bind(GLuint shaderProgram, unsigned int id);
	std::string getFileName() const;
	static Texture* getFromLink(const std::string& textureLink, const std::vector<std::unique_ptr<Texture>>& allTextures);
protected:
	GLuint textureID;
	GLuint texhandle;
private:
	unsigned char* loadTexture();
	void deleteTexture(unsigned char* imgdata);
	int width, height, comps;
	std::string filename;
};

class NormalMap: public Texture
{
public:
	NormalMap(std::string filename = "../../../res/runtime/normalmaps/default_normalmap.jpg");
	void bind(GLuint shaderProgram, unsigned int id);
	static NormalMap* getFromLink(const std::string& normalMapLink, const std::vector<std::unique_ptr<NormalMap>>& allNormalMaps);
private:
};

class ParallaxMap: public Texture
{
public:
	ParallaxMap(std::string filename = "../../../res/runtime/parallaxmaps/default_parallax.png");
	void bind(GLuint shaderProgram, unsigned int id);
	static ParallaxMap* getFromLink(const std::string& parallaxMapLink, const std::vector<std::unique_ptr<ParallaxMap>>& allParallaxMaps);
private:
};

class CubeMap
{
public:
	CubeMap(const std::string& rightTexture, const std::string& leftTexture, const std::string& topTexture, const std::string& bottomTexture, const std::string& backTexture, const std::string& frontTexture);
	~CubeMap();
	void bind(GLuint shaderProgram, unsigned int id);
private:
	std::vector<unsigned char*> loadTextures();
	GLuint texHandle, textureID;
	const std::string &rightTexture, &leftTexture, &topTexture, &bottomTexture, &backTexture, &frontTexture;
	int width[6], height[6], comps[6];
};

#endif