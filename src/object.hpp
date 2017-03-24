#ifndef OBJECT_HPP
#define OBJECT_HPP
#include "quaternion.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"

class Object
{
public:
	Object(std::string meshLink, std::string textureLink, std::string displacementMapLink, std::string normalMapLink, Vector3F pos, Vector3F rot, Vector3F scale);
	Vector3F getPos() const;
	Vector3F getRot() const;
	Vector3F getScale() const;
	
	Vector3F& getPosR();
	Vector3F& getRotR();
	Vector3F& getScaleR();
	
	std::string getMeshLink() const;
	std::string getTextureLink() const;
	std::string getNormalMapLink() const;
	std::string getDisplacementMapLink() const;
	
	void render(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Texture>& tex, const std::shared_ptr<NormalMap>& nm, const std::shared_ptr<DisplacementMap>& dm, const Camera& cam, const Shader& shad, float width, float height);
protected:
	float m[16], v[16], p[16];
	Vector3F pos, rot, scale;
	const std::string meshLink, textureLink, normalMapLink, displacementMapLink;
};

#endif