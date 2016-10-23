#include "object.hpp"

/*
Object::Object(Mesh mesh, Texture texture, Vector3F pos, Vector3F rot, Vector3F scale)
{
	this->mesh = mesh;
	this->tex = texture;
	this->pos = pos;
	this->rot = rot;
	this->scale = scale;
}
*/

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
/*
Mesh Object::getMesh()
{
	return (this->mesh);
}

Texture Object::getTexture()
{
	return (this->tex);
}
*/

void Object::render(Mesh* mesh, Texture* tex, Camera& cam, Shader& shad, float width, float height)
{
	//(MatrixTransformations::createMVPMatrix(this->pos, this->rot, this->scale, cam.getPosR(), cam.getRotR(), 1.5708, width, height, 0.1f, 1000.0f).washed()).fillData(this->mvp);
	(MatrixTransformations::createModelMatrix(this->pos, this->rot, this->scale).washed()).fillData(this->m);
	(MatrixTransformations::createViewMatrix(cam.getPosR(), cam.getRotR()).washed()).fillData(this->v);
	(MatrixTransformations::createProjectionMatrix(1.5708, width, height, 0.1f, 1000.0f).washed()).fillData(this->p);
	//std::cout << "Matrices generated, binding texture...\n";
	tex->bind(0);
	//std::cout << "Texture bound, pushing matrices to shader...\n";
	shad.update(this->m, this->v, this->p);
	//std::cout << "Matrices pushed, rendering the mesh... (This mesh uses the filename '" << mesh->getFileName() << "'.\n";
	mesh->render();
	//std::cout << "Sucessfully rendered the object.\n";
}