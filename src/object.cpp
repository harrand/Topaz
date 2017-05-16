#include "object.hpp"

Object::Object(std::string meshLink, std::vector<std::pair<std::string, Texture::TextureType>> textures, Vector3F pos, Vector3F rot, Vector3F scale): pos(pos), rot(rot), scale(scale), meshLink(meshLink), textures(textures){}

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

const std::vector<std::pair<std::string, Texture::TextureType>> Object::getTextures() const
{
	return this->textures;
}

void Object::render(Mesh* mesh, Texture* tex, NormalMap* nm, ParallaxMap* pm, DisplacementMap* dm, const Camera& cam, const Shader& shad, float width, float height) const
{
	if(mesh == nullptr)
		return;
	shad.bind();
	if(tex != nullptr)
		tex->bind(shad.getProgramHandle(), static_cast<unsigned int>(tex->getTextureType()));
	if(nm != nullptr)
		nm->bind(shad.getProgramHandle(), static_cast<unsigned int>(nm->getTextureType()));
	if(pm != nullptr)
		pm->bind(shad.getProgramHandle(), static_cast<unsigned int>(pm->getTextureType()));
	if(dm != nullptr)
		dm->bind(shad.getProgramHandle(), static_cast<unsigned int>(dm->getTextureType()));
	shad.update(MatrixTransformations::createModelMatrix(this->pos, this->rot, this->scale).fillData(), MatrixTransformations::createViewMatrix(cam.getPos(), cam.getRot()).fillData(), MatrixTransformations::createProjectionMatrix(1.5708, width, height, 0.1f, 10000.0f).fillData());
	//glFrontFace(GL_CCW);
	mesh->render();
	//glFrontFace(GL_CW);
}

Skybox::Skybox(std::string cubeMeshLink, CubeMap& cm): cubeMeshLink(cubeMeshLink), cm(cm){}

void Skybox::render(const Camera& cam, const Shader& shad, const std::vector<std::unique_ptr<Mesh>>& allMeshes, float width, float height)
{
	shad.bind();
	this->cm.bind(shad.getProgramHandle(), 0);
	shad.update(MatrixTransformations::createModelMatrix(cam.getPos(), Vector3F(), Vector3F(10000, 10000, 10000)).fillData(), MatrixTransformations::createViewMatrix(cam.getPos(), cam.getRot()).fillData(), MatrixTransformations::createProjectionMatrix(1.5708, width, height, 0.1f, 20000).fillData());
	glFrontFace(GL_CW);
	Mesh::getFromLink(this->cubeMeshLink, allMeshes)->render();
	glFrontFace(GL_CCW);
}