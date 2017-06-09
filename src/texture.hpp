#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include <memory>
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "glew.h"
#include "utility.hpp"

class FrameBuffer
{
public:
	FrameBuffer(unsigned int width = 256, unsigned int height = 256);
	FrameBuffer(const FrameBuffer& copy) = default;
	FrameBuffer(FrameBuffer&& move) = default;
	FrameBuffer& operator=(const FrameBuffer& rhs) = default;
	
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
	// NormalMap and ParallaxMap inherit copy and move constructors so having them defined as default still works fine.
	Texture(const Texture& copy);
	Texture(Texture&& move);
	Texture& operator=(const Texture& rhs) = delete;
	
	~Texture();
	void bind(GLuint shaderProgram, unsigned int id);
	std::string getFileName() const;
	
	enum class TextureType : unsigned int
	{
		TEXTURE = 0,
		NORMAL_MAP = 1,
		PARALLAX_MAP = 2,
		DISPLACEMENT_MAP = 3,
		TEXTURE_TYPES = 4,
	};
	
	virtual TextureType getTextureType();
	
	template<class Tex>
	static Tex* getFromLink(const std::string& textureLink, const std::vector<std::unique_ptr<Tex>>& allTextures)
	{
		for(auto& texture : allTextures)
		{
			if(texture->getFileName() == textureLink)
				return texture.get();
		}
		return nullptr;
	}
protected:
	unsigned char* loadTexture();
	void deleteTexture(unsigned char* imgdata);
	std::string filename;
	GLuint textureID;
	GLuint texhandle;
	int width, height, comps;
};

class NormalMap: public Texture
{
public:
	NormalMap(std::string filename = "../../../res/runtime/normalmaps/default_normalmap.jpg");
	NormalMap(const NormalMap& copy) = default;
	NormalMap(NormalMap&& move) = default;
	NormalMap& operator=(const NormalMap& rhs) = delete;
	
	void bind(GLuint shaderProgram, unsigned int id);
	TextureType getTextureType();
	
	//static NormalMap* getFromLink(const std::string& normalMapLink, const std::vector<std::unique_ptr<NormalMap>>& allNormalMaps);
};

class ParallaxMap: public Texture
{
public:
	ParallaxMap(std::string filename = "../../../res/runtime/parallaxmaps/default_parallax.png");
	ParallaxMap(const ParallaxMap& copy) = default;
	ParallaxMap(ParallaxMap&& move) = default;
	ParallaxMap& operator=(const ParallaxMap& rhs) = delete;
	
	void bind(GLuint shaderProgram, unsigned int id);
	TextureType getTextureType();
	
	//static ParallaxMap* getFromLink(const std::string& parallaxMapLink, const std::vector<std::unique_ptr<ParallaxMap>>& allParallaxMaps);
};

class DisplacementMap: public Texture
{
public:
	DisplacementMap(std::string filename = "../../../res/runtime/displacementmaps/default_displacement.png");
	DisplacementMap(const DisplacementMap& copy) = default;
	DisplacementMap(DisplacementMap&& move) = default;
	DisplacementMap& operator=(const DisplacementMap& rhs) = delete;
	
	void bind(GLuint shaderProgram, unsigned int id);
	TextureType getTextureType();
	
	//static DisplacementMap* getFromLink(const std::string& DisplacementMapLink, const std::vector<std::unique_ptr<DisplacementMap>>& allDisplacementMaps);
};

class CubeMap
{
public:
	CubeMap(const std::string& rightTexture, const std::string& leftTexture, const std::string& topTexture, const std::string& bottomTexture, const std::string& backTexture, const std::string& frontTexture);
	CubeMap(const std::string& textureDirectory = "./", const std::string& skyboxName = "skybox", const std::string& imageExtension = ".png");
	//CubeMap greenhaze(texturesDirectory + "greenhaze_rt.png", texturesDirectory + "greenhaze_lf.png", texturesDirectory + "greenhaze_up.png", texturesDirectory + "greenhaze_dn.png", texturesDirectory + "greenhaze_bk.png", texturesDirectory + "greenhaze_ft.png");
	CubeMap(const CubeMap& copy);
	CubeMap(CubeMap&& move);
	CubeMap& operator=(const CubeMap& rhs) = delete;
	~CubeMap();
	
	void bind(GLuint shaderProgram, unsigned int id);
private:
	std::vector<unsigned char*> loadTextures();
	GLuint texHandle, textureID;
	const std::string &rightTexture, &leftTexture, &topTexture, &bottomTexture, &backTexture, &frontTexture;
	int width[6], height[6], comps[6];
};

#endif