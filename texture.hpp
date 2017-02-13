#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include <string>
#include <cassert>
#include <iostream>
#include <vector>
#include "stb_image.h"
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "glew.h"

class Texture
{
public:
	Texture(std::string filename = "./res/textures/undefined.jpg");
	~Texture();
	void bind(unsigned int id);
	std::string getFileName();
	static Texture* getFromLink(std::string textureLink, std::vector<Texture*> allTextures);
private:
	unsigned char* loadTexture();
	void deleteTexture(unsigned char* imgdata);
	int width, height, comps;
	std::string filename;
	GLuint texhandle;
};

class ShadowMap: public Texture
{
public:
	ShadowMap(std::string filename = "./res/textures/undefined.jpg");
private:
};

#endif