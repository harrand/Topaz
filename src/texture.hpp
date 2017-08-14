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
	GLuint framebuffer_handle, texture_handle, depth_render_buffer_handle;
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
	GLuint texture_id;
	GLuint texture_handle;
	int width, height, components;
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
	CubeMap(std::string right_texture, std::string left_texture, std::string top_texture, std::string bottom_texture, std::string back_texture, std::string front_texture);
	CubeMap(std::string textureDirectory = "./", std::string skyboxName = "skybox", std::string imageExtension = ".png");
	//CubeMap greenhaze(texturesDirectory + "greenhaze_rt.png", texturesDirectory + "greenhaze_lf.png", texturesDirectory + "greenhaze_up.png", texturesDirectory + "greenhaze_dn.png", texturesDirectory + "greenhaze_bk.png", texturesDirectory + "greenhaze_ft.png");
	CubeMap(const CubeMap& copy);
	CubeMap(CubeMap&& move);
	CubeMap& operator=(const CubeMap& rhs) = delete;
	~CubeMap();
	
	void bind(GLuint shaderProgram, unsigned int id);
private:
	std::vector<unsigned char*> loadTextures();
	GLuint texture_handle, texture_id;
	const std::string right_texture, left_texture, top_texture, bottom_texture, back_texture, front_texture;
	int width[6], height[6], components[6];
};

#endif