#include "object.hpp"

Object::Object(std::string meshLink, std::string textureLink, std::string normalMapLink, std::string parallaxMapLink, Vector3F pos, Vector3F rot, Vector3F scale): meshLink(meshLink), textureLink(textureLink), normalMapLink(normalMapLink), parallaxMapLink(parallaxMapLink), pos(pos), rot(rot), scale(scale){}

const Vector3F& Object::getPos() const
{
	return this->pos;
}

const Vector3F& Object::getRot() const
{
	return this->rot;
}

const Vector3F& Object::getScale() const
{
	return this->scale;
}

Vector3F& Object::getPosR()
{
	return this->pos;
}

Vector3F& Object::getRotR()
{
	return this->rot;
}

Vector3F& Object::getScaleR()
{
	return this->scale;
}

const std::string& Object::getMeshLink() const
{
	return this->meshLink;
}

const std::string& Object::getTextureLink() const
{
	return this->textureLink;
}

const std::string& Object::getNormalMapLink() const
{
	return this->normalMapLink;
}

const std::string& Object::getParallaxMapLink() const
{
	return this->parallaxMapLink;
}

void Object::render(Mesh* mesh, Texture* tex, NormalMap* nm, ParallaxMap* pm, const Camera& cam, const Shader& shad, float width, float height)
{
	if(mesh == NULL || tex == NULL || nm == NULL || pm == NULL)
	{
		return;
	}
	shad.bind();
	tex->bind(shad.getProgramHandle(), 0);
	nm->bind(shad.getProgramHandle(), 1);
	pm->bind(shad.getProgramHandle(), 2);
	shad.update(MatrixTransformations::createModelMatrix(this->pos, this->rot, this->scale).fillData(), MatrixTransformations::createViewMatrix(cam.getPos(), cam.getRot()).fillData(), MatrixTransformations::createProjectionMatrix(1.5708, width, height, 0.1f, 10000.0f).fillData());
	mesh->render();
}