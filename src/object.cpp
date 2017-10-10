#include "object.hpp"

Object::Object(std::string mesh_link, std::vector<std::pair<std::string, Texture::TextureType>> textures, Vector3F pos, Vector3F rot, Vector3F scale, unsigned int shininess, float parallax_map_scale, float parallax_map_offset, float displacement_factor): pos(std::move(pos)), rot(std::move(rot)), scale(std::move(scale)), shininess(shininess), parallax_map_scale(parallax_map_scale), parallax_map_offset(parallax_map_offset), displacement_factor(displacement_factor), mesh_link(std::move(mesh_link)), textures(std::move(textures)){}

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

unsigned int Object::getShininess() const
{
	return this->shininess;
}

float Object::getParallaxMapScale() const
{
	return this->parallax_map_scale;
}

float Object::getParallaxMapOffset() const
{
	return this->parallax_map_offset;
}

float Object::getDisplacementFactor() const
{
	return this->displacement_factor;
}

void Object::setPosition(Vector3F position)
{
	this->pos = position;
}

void Object::setRotation(Vector3F rotation)
{
	this->rot = rotation;
}

void Object::setScale(Vector3F scale)
{
	this->scale = scale;
}

void Object::setShininess(float shininess)
{
	this->shininess = shininess;
}

void Object::setParallaxMapScale(float parallax_map_scale)
{
	this->parallax_map_scale = parallax_map_scale;
}

void Object::setParallaxMapOffset(float parallax_map_offset)
{
	this->parallax_map_offset = parallax_map_offset;
}

void Object::setDisplacementFactor(float displacement_factor)
{
	this->displacement_factor = displacement_factor;
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
	shad.update(Matrix4x4::createModelMatrix(this->pos, this->rot, this->scale).fillData(), Matrix4x4::createViewMatrix(cam.getPosition(), cam.getRotation()).fillData(), Matrix4x4::createPerspectiveMatrix(cam.getFOV(), width, height, cam.getNearClip(), cam.getFarClip()).fillData(), this->shininess, this->parallax_map_scale, this->parallax_map_offset, this->displacement_factor);
	//glFrontFace(GL_CCW);
	mesh->render(shad.hasTessellationControlShader());
	//glFrontFace(GL_CW);
}

Skybox::Skybox(std::string cube_mesh_link, CubeMap& cm): cube_mesh_link(cube_mesh_link), cm(cm){}

void Skybox::render(const Camera& cam, const Shader& shad, const std::vector<std::unique_ptr<Mesh>>& all_meshes, float width, float height)
{
	shad.bind();
	this->cm.bind(shad.getProgramHandle(), 0);
	shad.update(Matrix4x4::createModelMatrix(cam.getPosition(), Vector3F(), Vector3F(cam.getFarClip(), cam.getFarClip(), cam.getFarClip())).fillData(), Matrix4x4::createViewMatrix(cam.getPosition(), cam.getRotation()).fillData(), Matrix4x4::createPerspectiveMatrix(cam.getFOV(), width, height, cam.getNearClip(), cam.getFarClip()).fillData(), 0, 0, 0, 0);
	glFrontFace(GL_CW);
	tz::graphics::findMesh(this->cube_mesh_link, all_meshes)->render(shad.hasTessellationControlShader());
	glFrontFace(GL_CCW);
}