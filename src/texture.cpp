#include "texture.hpp"

FrameBuffer::FrameBuffer(unsigned int width, unsigned int height): width(width), height(height), fbHandle(0)
{
	glGenFramebuffers(1, &this->fbHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbHandle);
	
	glGenTextures(1, &this->texHandle);
	glBindTexture(GL_TEXTURE_2D, this->texHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	
	//Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glGenRenderbuffers(1, &this->depthRenderBufferHandle);
	glBindRenderbuffer(GL_RENDERBUFFER, this->depthRenderBufferHandle);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->depthRenderBufferHandle);
	
	//Configure framebuffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->texHandle, 0);
	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers);
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		//problim
	}
}

void FrameBuffer::setRenderTarget() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbHandle);
	glViewport(0, 0, this->width, this->height);
}

void FrameBuffer::bind(unsigned int id) const
{
	if(id > 31 || id < 0)
	{
		LogUtility::error("FrameBuffer bind ID " + CastUtility::toString<unsigned int>(id) + " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texHandle);
}

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
		LogUtility::error("Texture from the path: '" + filename + "' could not be loaded.");
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

Texture::Texture(const Texture& copy): Texture(copy.getFileName()){}

Texture::Texture(Texture&& move): filename(move.getFileName()), textureID(move.textureID), texhandle(move.texhandle), width(move.width), height(move.height), comps(move.comps)
{
	move.textureID = 0;
	move.texhandle = 0;
}

Texture::~Texture()
{
	// glDeleteTextures silently ignores 0 as a texhandle so this should not cause problems if just moved.
	glDeleteTextures(1, &(this->texhandle));
}

void Texture::bind(GLuint shaderProgram, unsigned int id)
{
	if(id > 31 || id < 0)
	{
		LogUtility::error("FrameBuffer bind ID " + CastUtility::toString<unsigned int>(id) + " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	this->textureID = glGetUniformLocation(shaderProgram, "textureSampler");
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texhandle);
	glUniform1i(this->textureID, id);
}

std::string Texture::getFileName() const
{
	return this->filename;
}

//static
Texture* Texture::getFromLink(const std::string& textureLink, const std::vector<std::unique_ptr<Texture>>& allTextures)
{
	for(unsigned int i = 0; i < allTextures.size(); i++)
	{
		if(allTextures.at(i)->getFileName() == textureLink)
			return allTextures.at(i).get();
	}
	return NULL;
}

NormalMap::NormalMap(std::string filename): Texture(filename){}

void NormalMap::bind(GLuint shaderProgram, unsigned int id)
{
	if(id > 31 || id < 0)
	{
		LogUtility::error("FrameBuffer bind ID " + CastUtility::toString<unsigned int>(id) + " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	this->textureID = glGetUniformLocation(shaderProgram, "normalMapSampler");
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texhandle);
	glUniform1i(this->textureID, id);
}

//static
NormalMap* NormalMap::getFromLink(const std::string& normalMapLink, const std::vector<std::unique_ptr<NormalMap>>& allNormalMaps)
{
	for(unsigned int i = 0; i < allNormalMaps.size(); i++)
	{
		if(allNormalMaps.at(i)->getFileName() == normalMapLink)
			return allNormalMaps.at(i).get();
	}
	return NULL;
}

ParallaxMap::ParallaxMap(std::string filename): Texture(filename){}

void ParallaxMap::bind(GLuint shaderProgram, unsigned int id)
{
	if(id > 31 || id < 0)
	{
		LogUtility::error("FrameBuffer bind ID " + CastUtility::toString<unsigned int>(id) + " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	this->textureID = glGetUniformLocation(shaderProgram, "parallaxMapSampler");
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texhandle);
	glUniform1i(this->textureID, id);
}

//static
ParallaxMap* ParallaxMap::getFromLink(const std::string& parallaxMapLink, const std::vector<std::unique_ptr<ParallaxMap>>& allParallaxMaps)
{
	for(unsigned int i = 0; i < allParallaxMaps.size(); i++)
	{
		if(allParallaxMaps.at(i)->getFileName() == parallaxMapLink)
			return allParallaxMaps.at(i).get();
	}
	return NULL;
}

CubeMap::CubeMap(const std::string& rightTexture, const std::string& leftTexture, const std::string& topTexture, const std::string& bottomTexture, const std::string& backTexture, const std::string& frontTexture): rightTexture(rightTexture), leftTexture(leftTexture), topTexture(topTexture), bottomTexture(bottomTexture), backTexture(backTexture), frontTexture(frontTexture)
{
	glGenTextures(1, &(this->texHandle));
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->texHandle);
	std::vector<unsigned char*> faceData = this->loadTextures();
	for(GLuint i = 0; i < faceData.size(); i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, this->width[(unsigned int)i], this->height[(unsigned int)i], 0, GL_RGBA, GL_UNSIGNED_BYTE, faceData.at(i));
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	for(auto& data : faceData)
		stbi_image_free(data);
}

CubeMap::CubeMap(const std::string& textureDirectory, const std::string& skyboxName, const std::string& imageExtension): CubeMap(textureDirectory + skyboxName + "_rt" + imageExtension, textureDirectory + skyboxName + "_lf" + imageExtension, textureDirectory + skyboxName + "_up" + imageExtension, textureDirectory + skyboxName + "_dn" + imageExtension, textureDirectory + skyboxName + "_bk" + imageExtension, textureDirectory + skyboxName + "_ft" + imageExtension){}

CubeMap::CubeMap(const CubeMap& copy): CubeMap(copy.rightTexture, copy.leftTexture, copy.topTexture, copy.bottomTexture, copy.backTexture, copy.frontTexture){}

CubeMap::CubeMap(CubeMap&& move): texHandle(move.texHandle), textureID(move.textureID), rightTexture(move.rightTexture), leftTexture(move.leftTexture), topTexture(move.topTexture), bottomTexture(move.bottomTexture), backTexture(move.backTexture), frontTexture(move.frontTexture)
{
	for(unsigned int i = 0; i < 6; i++)
	{
		this->width[i] = move.width[i];
		this->height[i] = move.height[i];
		this->comps[i] = move.comps[i];
	}
	move.texHandle = 0;
	move.textureID = 0;
}

CubeMap::~CubeMap()
{
	glDeleteTextures(1, &(this->texHandle));
}

void CubeMap::bind(GLuint shaderProgram, unsigned int id)
{
	if(id > 31 || id < 0)
	{
		LogUtility::error("FrameBuffer bind ID " + CastUtility::toString<unsigned int>(id) + " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	this->textureID = glGetUniformLocation(shaderProgram, "cubeMapSampler");
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->texHandle);
	glUniform1i(this->textureID, id);
}

std::vector<unsigned char*> CubeMap::loadTextures()
{
	std::vector<unsigned char*> imageData;
	imageData.reserve(6);
	imageData.push_back(stbi_load((this->rightTexture).c_str(), &(this->width[0]), &(this->height[0]), &(this->comps[0]), 4));
	imageData.push_back(stbi_load((this->leftTexture).c_str(), &(this->width[1]), &(this->height[1]), &(this->comps[1]), 4));
	imageData.push_back(stbi_load((this->topTexture).c_str(), &(this->width[2]), &(this->height[2]), &(this->comps[2]), 4));
	imageData.push_back(stbi_load((this->bottomTexture).c_str(), &(this->width[3]), &(this->height[3]), &(this->comps[3]), 4));
	imageData.push_back(stbi_load((this->backTexture).c_str(), &(this->width[4]), &(this->height[4]), &(this->comps[4]), 4));
	imageData.push_back(stbi_load((this->frontTexture).c_str(), &(this->width[5]), &(this->height[5]), &(this->comps[5]), 4));
	return imageData;
}
