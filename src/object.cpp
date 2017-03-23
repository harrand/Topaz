#include "object.hpp"

Object::Object(std::string meshLink, std::string textureLink, std::string normalMapLink, Vector3F pos, Vector3F rot, Vector3F scale): meshLink(meshLink), textureLink(textureLink), normalMapLink(normalMapLink), pos(pos), rot(rot), scale(scale){}

Vector3F Object::getPos() const
{
	return this->pos;
}

Vector3F Object::getRot() const
{
	return this->rot;
}

Vector3F Object::getScale() const
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

std::string Object::getMeshLink() const
{
	return this->meshLink;
}

std::string Object::getTextureLink() const
{
	return this->textureLink;
}

std::string Object::getNormalMapLink() const
{
	return this->normalMapLink;
}

void Object::render(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> tex, std::shared_ptr<NormalMap> nm, Camera& cam, Shader& shad, float width, float height)
{
	(MatrixTransformations::createQuaternionSourcedModelMatrix(this->pos, this->rot, this->scale).washed()).fillData(this->m);
	//(MatrixTransformations::createViewMatrix(cam.getPosR(), cam.getRotR()).washed()).fillData(this->v);
	MatrixTransformations::createQuaternionSourcedViewMatrix(cam.getPosR(), cam.getRotR()).washed().fillData(this->v);
	(MatrixTransformations::createProjectionMatrix(1.5708, width, height, 0.1f, 1000.0f).washed()).fillData(this->p);
	shad.bind();
	tex->bind(shad.getProgramHandle(), 0);
	nm->bind(shad.getProgramHandle(), 1);
	shad.update(this->m, this->v, this->p);
	mesh->render();
}