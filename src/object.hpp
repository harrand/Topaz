#ifndef OBJECT_HPP
#define OBJECT_HPP
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include <initializer_list>

class Object
{
public:
	Object(std::string meshLink, std::vector<std::pair<std::string, Texture::TextureType>> textures, Vector3F pos, Vector3F rot, Vector3F scale);
	Object(const Object& copy) = default;
	Object(Object&& move) = default;
	Object& operator=(const Object& rhs) = default;
	
	const Vector3F& getPosition() const;
	const Vector3F& getRotation() const;
	const Vector3F& getScale() const;
	Vector3F& getPositionR();
	Vector3F& getRotationR();
	Vector3F& getScaleR();
	
	const std::string& getMeshLink() const;
	const std::vector<std::pair<std::string, Texture::TextureType>> getTextures() const;
	
	virtual void render(Mesh* mesh, Texture* tex, NormalMap* nm, ParallaxMap* pm, DisplacementMap* dm, const Camera& cam, const Shader& shad, float width, float height) const;
protected:
	Vector3F pos, rot, scale;
	std::string meshLink;
	std::vector<std::pair<std::string, Texture::TextureType>> textures;
};

class Skybox
{
public:
	Skybox(std::string cubeMeshLink, CubeMap& cm);
	void render(const Camera& cam, const Shader& shad, const std::vector<std::unique_ptr<Mesh>>& allMeshes, float width, float height);
private:
	std::string cubeMeshLink;
	CubeMap& cm;
};

#endif