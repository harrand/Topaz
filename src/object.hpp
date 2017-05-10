#ifndef OBJECT_HPP
#define OBJECT_HPP
#include "matrix.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"

class Object
{
public:
	Object(std::string meshLink, std::string textureLink, std::string parallaxMapLink, std::string normalMapLink, Vector3F pos, Vector3F rot, Vector3F scale);
	Object(const Object& copy) = default;
	Object(Object&& move) = default;
	Object& operator=(const Object& rhs) = default;
	
	const Vector3F& getPos() const;
	const Vector3F& getRot() const;
	const Vector3F& getScale() const;
	Vector3F& getPosR();
	Vector3F& getRotR();
	Vector3F& getScaleR();
	
	const std::string& getMeshLink() const;
	const std::string& getTextureLink() const;
	const std::string& getNormalMapLink() const;
	const std::string& getParallaxMapLink() const;
	
	virtual void render(Mesh* mesh, Texture* tex, NormalMap* nm, ParallaxMap* pm, const Camera& cam, const Shader& shad, float width, float height) const;
protected:
	Vector3F pos, rot, scale;
	std::string meshLink, textureLink, normalMapLink, parallaxMapLink;
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