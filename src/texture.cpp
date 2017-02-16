#include "texture.hpp"

unsigned char* Texture::loadTexture()
{
	return stbi_load((this->filename).c_str(), &(this->width), &(this->height), &(this->comps), 4);
}

// Deleting texture as far as stb_image is concerned (We want to leave it alone when it's gone to the GPU)
void Texture::deleteTexture(unsigned char* imgdata)
{
	stbi_image_free(imgdata);
}

Texture::Texture(std::string filename)
{
	this->filename = filename;
	unsigned char* imgdata = this->loadTexture();
	
	if(imgdata == NULL)
	{
		std::cerr << "Texture from the path: '" << filename << "' couldn't be loaded!\n";
	}
	
	//Store tex data in the handle
	glGenTextures(1, &(this->texhandle));
	// Let OGL know it's just a 2d texture.
	glBindTexture(GL_TEXTURE_2D, this->texhandle);
	
	// These both are optional.
	// Controls texture wrapping. i.e it wraps if dimensions of model is larger than image size.
	// GL_REPEAT means that we're just going to repeat the beginning of the image for the wrap.
	// Another is GL_CLAMP, which just fills the remainder with a colour (Probably black)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	// These both are also optional
	// 512x512 texture for example. We're almost definitely not gonna render exactly 512x512 pixels.
	// Especially once we use rotational matrices and rotation in general.
	// MIN is when the texture takes up fewer pixels, MAG is when it takes up more.
	// GL_LINEAR is when it linearly interpolates the pixels to produce the most accurate.
	// Another solution is GL_NEAREST which doesnt filter, it just takes the pixel sampled anyway
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// Like glBufferData, just sends the image data to the GPU
	// Par2 is mipmapping, when we change the resolution of the image depending on how far we are away from it (i.e higher resolution if we're closer)
	// Expand upon this later, 0 means that we're not changing anything.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, /*Border size*/0, GL_RGBA, GL_UNSIGNED_BYTE, imgdata);
	
	this->deleteTexture(imgdata);
}

Texture::~Texture()
{
	glDeleteTextures(1, &(this->texhandle));
}

void Texture::bind(unsigned int id)
{
	assert(id >= 0 && id <= 31);
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texhandle);
}

std::string Texture::getFileName()
{
	return this->filename;
}

//static
std::shared_ptr<Texture> Texture::getFromLink(std::string textureLink, std::vector<std::shared_ptr<Texture>> allTextures)
{
	for(unsigned int i = 0; i < allTextures.size(); i++)
	{
		if(allTextures.at(i)->getFileName() == textureLink)
			return allTextures.at(i);
	}
	return __null;
}

ShadowMap::ShadowMap(std::string filename)
{
	
}