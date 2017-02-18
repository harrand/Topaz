#include "object.hpp"

Object::Object(std::string meshLink, std::string textureLink, Vector3F pos, Vector3F rot, Vector3F scale)
{
	this->meshLink = meshLink;
	this->textureLink = textureLink;
	this->pos = pos;
	this->rot = rot;
	this->scale = scale;
}

Vector3F Object::getPos()
{
	return this->pos;
}

Vector3F Object::getRot()
{
	return this->rot;
}

Vector3F Object::getScale()
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

std::string Object::getMeshLink()
{
	return this->meshLink;
}

std::string Object::getTextureLink()
{
	return this->textureLink;
}

void Object::render(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> tex, Camera& cam, Shader& shad, float width, float height)
{
	(MatrixTransformations::createModelMatrix(this->pos, this->rot, this->scale).washed()).fillData(this->m);
	(MatrixTransformations::createViewMatrix(cam.getPosR(), cam.getRotR()).washed()).fillData(this->v);
	(MatrixTransformations::createProjectionMatrix(1.5708, width, height, 0.1f, 1000.0f).washed()).fillData(this->p);
	tex->bind(0);
	shad.update(this->m, this->v, this->p);
	mesh->render();
}