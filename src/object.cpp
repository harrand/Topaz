#include "object.hpp"

Object::Object(std::string meshLink, std::string textureLink, std::string normalMapLink, std::string parallaxMapLink, Vector3F pos, Vector3F rot, Vector3F scale): pos(pos), rot(rot), scale(scale), meshLink(meshLink), textureLink(textureLink), normalMapLink(normalMapLink), parallaxMapLink(parallaxMapLink){}

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

void Object::render(Mesh* mesh, Texture* tex, NormalMap* nm, ParallaxMap* pm, const Camera& cam, const Shader& shad, float width, float height) const
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
	glFrontFace(GL_CCW);
	mesh->render();
	glFrontFace(GL_CW);
}

Skybox::Skybox(std::string cubeMeshLink, CubeMap& cm): cubeMeshLink(cubeMeshLink), cm(cm){}

void Skybox::render(const Camera& cam, const Shader& shad, const std::vector<std::unique_ptr<Mesh>>& allMeshes, float width, float height)
{
	shad.bind();
	this->cm.bind(shad.getProgramHandle(), 0);
	shad.update(MatrixTransformations::createModelMatrix(cam.getPos(), Vector3F(), Vector3F(10000, 10000, 10000)).fillData(), MatrixTransformations::createViewMatrix(cam.getPos(), cam.getRot()).fillData(), MatrixTransformations::createProjectionMatrix(1.5708, width, height, 0.1f, 20000).fillData());
	Mesh::getFromLink(this->cubeMeshLink, allMeshes)->render();
}