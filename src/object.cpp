#include "object.hpp"

StaticObject::StaticObject(std::string mesh_link, std::vector<std::pair<std::string, Texture::TextureType>> textures, Vector3F pos, Vector3F rot, Vector3F scale, unsigned int shininess, float parallax_map_scale, float parallax_map_offset, float displacement_factor): pos(std::move(pos)), rot(std::move(rot)), scale(std::move(scale)), shininess(shininess), parallax_map_scale(parallax_map_scale), parallax_map_offset(parallax_map_offset), displacement_factor(displacement_factor), mesh_link(std::move(mesh_link)), textures(std::move(textures)){}

const Vector3F& StaticObject::getPosition() const
{
	return this->pos;
}

const Vector3F& StaticObject::getRotation() const
{
	return this->rot;
}

const Vector3F& StaticObject::getScale() const
{
	return this->scale;
}

unsigned int StaticObject::getShininess() const
{
	return this->shininess;
}

float StaticObject::getParallaxMapScale() const
{
	return this->parallax_map_scale;
}

float StaticObject::getParallaxMapOffset() const
{
	return this->parallax_map_offset;
}

float StaticObject::getDisplacementFactor() const
{
	return this->displacement_factor;
}

Vector3F& StaticObject::getPositionR()
{
	return this->pos;
}

Vector3F& StaticObject::getRotationR()
{
	return this->rot;
}

Vector3F& StaticObject::getScaleR()
{
	return this->scale;
}

unsigned int& StaticObject::getShininessR()
{
	return this->shininess;
}

float& StaticObject::getParallaxMapScaleR()
{
	return this->parallax_map_scale;
}

float& StaticObject::getParallaxMapOffsetR()
{
	return this->parallax_map_offset;
}

float& StaticObject::getDisplacementFactorR()
{
	return this->displacement_factor;
}

const std::string& StaticObject::getMeshLink() const
{
	return this->mesh_link;
}

const std::vector<std::pair<std::string, Texture::TextureType>> StaticObject::getTextures() const
{
	return this->textures;
}

void StaticObject::render(Mesh* mesh, Texture* tex, NormalMap* nm, ParallaxMap* pm, DisplacementMap* dm, const Camera& cam, const Shader& shad, float width, float height) const
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
	shad.update(Matrix4x4::createModelMatrix(cam.getPosition(), Vector3F(), Vector3F(10000, 10000, 10000)).fillData(), Matrix4x4::createViewMatrix(cam.getPosition(), cam.getRotation()).fillData(), Matrix4x4::createPerspectiveMatrix(cam.getFOV(), width, height, cam.getNearClip(), cam.getFarClip()).fillData(), 0, 0, 0, 0);
	glFrontFace(GL_CW);
	tz::graphics::findMesh(this->cube_mesh_link, all_meshes)->render(shad.hasTessellationControlShader());
	glFrontFace(GL_CCW);
}

BoundingSphere tz::physics::boundSphere(const StaticObject& object, const std::vector<std::unique_ptr<Mesh>>& all_meshes)
{
	std::vector<Vector3F> positions_worldspace;
	const std::vector<Vector3F>& positions_modelspace = tz::graphics::findMesh(object.getMeshLink(), all_meshes)->getPositions();
	positions_worldspace.reserve(positions_modelspace.size());
	for(Vector3F position_modelspace : positions_modelspace)
	{
		Vector4F position_modelspace_homogeneous(position_modelspace.getX(), position_modelspace.getY(), position_modelspace.getZ(), 1.0f);
		Vector4F position_worldspace_homogeneous = Matrix4x4::createModelMatrix(object.getPosition(), object.getRotation(), object.getScale()) * position_modelspace_homogeneous;
		positions_worldspace.emplace_back(position_worldspace_homogeneous.getX(), position_worldspace_homogeneous.getY(), position_worldspace_homogeneous.getZ());
	}
	// Objects are static so don't need to worry about updating this ever again. Best solution is O(n) which is to find the average position and use that as centre, and then find maximum distance and make that the distance
	Vector3F mean = std::accumulate(positions_worldspace.begin(), positions_worldspace.end(), Vector3F(), [](const Vector3F& a, const Vector3F& b) -> Vector3F{return a + b;}) / positions_worldspace.size();
	// if centre is the mean, then distance is the distance between the mean and whatever is furthest away from the mean i.e biggest outlier
	std::transform(positions_worldspace.begin(), positions_worldspace.end(), positions_worldspace.begin(), [&mean](const Vector3F& position) -> Vector3F{return mean - position;});
	std::vector<float> distances;
	distances.reserve(positions_worldspace.size());
	for(auto displacement : positions_worldspace)
		distances.push_back(std::fabs(displacement.length()));
	return {mean, *std::max_element(distances.begin(), distances.end())};
}
	
AABB tz::physics::boundAABB(const StaticObject& object, const std::vector<std::unique_ptr<Mesh>>& all_meshes)
{
	const std::vector<Vector3F>& positions_modelspace = tz::graphics::findMesh(object.getMeshLink(), all_meshes)->getPositions();
	std::vector<Vector3F> positions_worldspace;
	positions_worldspace.reserve(positions_modelspace.size());
	for(Vector3F position_modelspace : positions_modelspace)
	{
		Vector4F position_modelspace_homogeneous(position_modelspace.getX(), position_modelspace.getY(), position_modelspace.getZ(), 1.0f);
		Vector4F position_worldspace_homogeneous = Matrix4x4::createModelMatrix(object.getPosition(), object.getRotation(), object.getScale()) * position_modelspace_homogeneous;
		positions_worldspace.emplace_back(position_worldspace_homogeneous.getX(), position_worldspace_homogeneous.getY(), position_worldspace_homogeneous.getZ());
	}
	constexpr float float_min = std::numeric_limits<float>::lowest();
	constexpr float float_max = std::numeric_limits<float>::max();
	float min_x = float_max, min_y = float_max, min_z = float_max;
	float max_x = float_min, max_y = float_min, max_z = float_min;
	for(const Vector3F& position_worldspace : positions_worldspace)
	{
		min_x = std::min(position_worldspace.getX(), min_x);
		min_y = std::min(position_worldspace.getY(), min_y);
		min_z = std::min(position_worldspace.getZ(), min_z);
		max_x = std::max(position_worldspace.getX(), max_x);
		max_y = std::max(position_worldspace.getY(), max_y);
		max_z = std::max(position_worldspace.getZ(), max_z);
	}
	return {Vector3F(min_x, min_y, min_z), Vector3F(max_x, max_y, max_z)};
}