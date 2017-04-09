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
	Object(std::string meshLink, std::string textureLink, std::string parallaxMapLink, std::string normalMapLink, Vector3F pos, Vector3F rot, Vector3F scale);
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
	
	void render(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Texture>& tex, const std::shared_ptr<NormalMap>& nm, const std::shared_ptr<ParallaxMap>& pm, const Camera& cam, const Shader& shad, float width, float height);
protected:
	Vector3F pos, rot, scale;
	const std::string meshLink, textureLink, normalMapLink, parallaxMapLink;
};

#endif