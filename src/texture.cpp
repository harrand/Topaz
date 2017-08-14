#include "texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

FrameBuffer::FrameBuffer(unsigned int width, unsigned int height): width(width), height(height), framebuffer_handle(0)
{
	glGenFramebuffers(1, &this->framebuffer_handle);
	glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_handle);
	
	glGenTextures(1, &this->texture_handle);
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	
	//Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glGenRenderbuffers(1, &this->depth_render_buffer_handle);
	glBindRenderbuffer(GL_RENDERBUFFER, this->depth_render_buffer_handle);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->depth_render_buffer_handle);
	
	//Configure framebuffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->texture_handle, 0);
	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers);
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		logutility::warning("FrameBuffer invalid; glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE");
	}
}

void FrameBuffer::setRenderTarget() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_handle);
	glViewport(0, 0, this->width, this->height);
}

void FrameBuffer::bind(unsigned int id) const
{
	if(id > 31)
	{
		logutility::error("FrameBuffer bind ID ", id, " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
}

unsigned char* Texture::loadTexture()
{
	return stbi_load((this->filename).c_str(), &(this->width), &(this->height), &(this->components), 4);
}

// Deleting texture as far as stb_image is concerned (We want to leave it alone when it's gone to the GPU)
void Texture::deleteTexture(unsigned char* imgdata)
{
	stbi_image_free(imgdata);
}

Texture::Texture(std::string filename): filename(std::move(filename))
{
	unsigned char* imgdata = this->loadTexture();
	if(imgdata == nullptr)
	{
		logutility::error("Texture from the path: '", filename, "' could not be loaded.");
	}
	
	//Store tex data in the handle
	glGenTextures(1, &(this->texture_handle));
	// Let OGL know it's just a 2d texture.
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	
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

Texture::Texture(const Texture& copy): Texture(copy.getFileName()){}

Texture::Texture(Texture&& move): filename(move.getFileName()), texture_id(move.texture_id), texture_handle(move.texture_handle), width(move.width), height(move.height), components(move.components)
{
	move.texture_id = 0;
	move.texture_handle = 0;
}

Texture::~Texture()
{
	// glDeleteTextures silently ignores 0 as a texture_handle so this should not cause problems if just moved.
	glDeleteTextures(1, &(this->texture_handle));
}

void Texture::bind(GLuint shaderProgram, unsigned int id)
{
	if(id > 31)
	{
		logutility::error("FrameBuffer bind ID ", id, " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	this->texture_id = glGetUniformLocation(shaderProgram, "textureSampler");
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	glUniform1i(this->texture_id, id);
}

std::string Texture::getFileName() const
{
	return this->filename;
}

Texture::TextureType Texture::getTextureType()
{
	return TextureType::TEXTURE;
}

NormalMap::NormalMap(std::string filename): Texture(filename){}

void NormalMap::bind(GLuint shaderProgram, unsigned int id)
{
	if(id > 31)
	{
		logutility::error("FrameBuffer bind ID ", id, " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	this->texture_id = glGetUniformLocation(shaderProgram, "normalMapSampler");
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	glUniform1i(this->texture_id, id);
}

Texture::TextureType NormalMap::getTextureType()
{
	return TextureType::NORMAL_MAP;
}

ParallaxMap::ParallaxMap(std::string filename): Texture(filename){}

void ParallaxMap::bind(GLuint shaderProgram, unsigned int id)
{
	if(id > 31)
	{
		logutility::error("FrameBuffer bind ID ", id, " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	this->texture_id = glGetUniformLocation(shaderProgram, "parallaxMapSampler");
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	glUniform1i(this->texture_id, id);
}

Texture::TextureType ParallaxMap::getTextureType()
{
	return TextureType::PARALLAX_MAP;
}

DisplacementMap::DisplacementMap(std::string filename): Texture(filename){}

void DisplacementMap::bind(GLuint shaderProgram, unsigned int id)
{
	if(id > 31)
	{
		logutility::error("FrameBuffer bind ID ", id, " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	this->texture_id = glGetUniformLocation(shaderProgram, "displacementMapSampler");
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	glUniform1i(this->texture_id, id);
}

Texture::TextureType DisplacementMap::getTextureType()
{
	return TextureType::DISPLACEMENT_MAP;
}

CubeMap::CubeMap(std::string right_texture, std::string left_texture, std::string top_texture, std::string bottom_texture, std::string back_texture, std::string front_texture): right_texture(std::move(right_texture)), left_texture(std::move(left_texture)), top_texture(std::move(top_texture)), bottom_texture(std::move(bottom_texture)), back_texture(std::move(back_texture)), front_texture(std::move(front_texture))
{
	glGenTextures(1, &(this->texture_handle));
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->texture_handle);
	std::vector<unsigned char*> faceData = this->loadTextures();
	for(GLuint i = 0; i < faceData.size(); i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, this->width[static_cast<unsigned int>(i)], this->height[static_cast<unsigned int>(i)], 0, GL_RGBA, GL_UNSIGNED_BYTE, faceData.at(i));
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	for(auto& data : faceData)
		stbi_image_free(data);
}

CubeMap::CubeMap(std::string textureDirectory, std::string skyboxName, std::string imageExtension): CubeMap(textureDirectory + skyboxName + "_rt" + imageExtension, textureDirectory + skyboxName + "_lf" + imageExtension, textureDirectory + skyboxName + "_up" + imageExtension, textureDirectory + skyboxName + "_dn" + imageExtension, textureDirectory + skyboxName + "_bk" + imageExtension, textureDirectory + skyboxName + "_ft" + imageExtension){}

CubeMap::CubeMap(const CubeMap& copy): CubeMap(copy.right_texture, copy.left_texture, copy.top_texture, copy.bottom_texture, copy.back_texture, copy.front_texture){}

CubeMap::CubeMap(CubeMap&& move): texture_handle(move.texture_handle), texture_id(move.texture_id), right_texture(move.right_texture), left_texture(move.left_texture), top_texture(move.top_texture), bottom_texture(move.bottom_texture), back_texture(move.back_texture), front_texture(move.front_texture)
{
	for(unsigned int i = 0; i < 6; i++)
	{
		this->width[i] = move.width[i];
		this->height[i] = move.height[i];
		this->components[i] = move.components[i];
	}
	move.texture_handle = 0;
	move.texture_id = 0;
}

CubeMap::~CubeMap()
{
	glDeleteTextures(1, &(this->texture_handle));
}

void CubeMap::bind(GLuint shaderProgram, unsigned int id)
{
	if(id > 31)
	{
		logutility::error("FrameBuffer bind ID ", id, " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	this->texture_id = glGetUniformLocation(shaderProgram, "cubeMapSampler");
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->texture_handle);
	glUniform1i(this->texture_id, id);
}

std::vector<unsigned char*> CubeMap::loadTextures()
{
	std::vector<unsigned char*> imageData;
	imageData.reserve(6);
	imageData.push_back(stbi_load((this->right_texture).c_str(), &(this->width[0]), &(this->height[0]), &(this->components[0]), 4));
	imageData.push_back(stbi_load((this->left_texture).c_str(), &(this->width[1]), &(this->height[1]), &(this->components[1]), 4));
	imageData.push_back(stbi_load((this->top_texture).c_str(), &(this->width[2]), &(this->height[2]), &(this->components[2]), 4));
	imageData.push_back(stbi_load((this->bottom_texture).c_str(), &(this->width[3]), &(this->height[3]), &(this->components[3]), 4));
	imageData.push_back(stbi_load((this->back_texture).c_str(), &(this->width[4]), &(this->height[4]), &(this->components[4]), 4));
	imageData.push_back(stbi_load((this->front_texture).c_str(), &(this->width[5]), &(this->height[5]), &(this->components[5]), 4));
	return imageData;
}
