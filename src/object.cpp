#include "object.hpp"
#include "quaternion.hpp"

Object::Object(std::string mesh_link, std::vector<std::pair<std::string, Texture::TextureType>> textures, Vector3F pos, Vector3F rot, Vector3F scale): pos(std::move(pos)), rot(std::move(rot)), scale(std::move(scale)), mesh_link(std::move(mesh_link)), textures(std::move(textures)){}

const Vector3F& Object::getPosition() const
{
	return this->pos;
}

const Vector3F& Object::getRotation() const
{
	return this->rot;
}

const Vector3F& Object::getScale() const
{
	return this->scale;
}

Vector3F& Object::getPositionR()
{
	return this->pos;
}

Vector3F& Object::getRotationR()
{
	return this->rot;
}

Vector3F& Object::getScaleR()
{
	return this->scale;
}

const std::string& Object::getMeshLink() const
{
	return this->mesh_link;
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
	shad.update(Matrix4x4::createModelMatrix(this->pos, this->rot, this->scale).fillData(), Matrix4x4::createViewMatrix(cam.getPosition(), cam.getRotation()).fillData(), Matrix4x4::createProjectionMatrix(1.5708, width, height, 0.1f, 10000.0f).fillData());
	//glFrontFace(GL_CCW);
	mesh->render(shad.hasTessellationControlShader());
	//glFrontFace(GL_CW);
}

Skybox::Skybox(std::string cube_mesh_link, CubeMap& cm): cube_mesh_link(cube_mesh_link), cm(cm){}

void Skybox::render(const Camera& cam, const Shader& shad, const std::vector<std::unique_ptr<Mesh>>& all_meshes, float width, float height)
{
	shad.bind();
	this->cm.bind(shad.getProgramHandle(), 0);
	shad.update(Matrix4x4::createModelMatrix(cam.getPosition(), Vector3F(), Vector3F(10000, 10000, 10000)).fillData(), Matrix4x4::createViewMatrix(cam.getPosition(), cam.getRotation()).fillData(), Matrix4x4::createProjectionMatrix(1.5708, width, height, 0.1f, 20000).fillData());
	glFrontFace(GL_CW);
	Mesh::getFromLink(this->cube_mesh_link, all_meshes)->render(shad.hasTessellationControlShader());
	glFrontFace(GL_CCW);
}