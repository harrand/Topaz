#include "physics/physics.hpp"
#include "graphics/static_object.hpp"

BoundingSphere tz::physics::bound_sphere(const Mesh& mesh)
{
	Vector3F mean_modelspace = std::accumulate(mesh.get_positions().begin(), mesh.get_positions().end(), Vector3F(), [](const Vector3F& a, const Vector3F& b) -> Vector3F{return a + b;}) / mesh.get_positions().size();
	std::vector<Vector3F> positions_modelspace = mesh.get_positions();
	std::transform(positions_modelspace.begin(), positions_modelspace.end(), positions_modelspace.begin(), [&mean_modelspace](const Vector3F& position) -> Vector3F{return mean_modelspace - position;});
	float max_distance = 0.0f;
	for(const auto& displacement : positions_modelspace)
		max_distance = std::max(displacement.length(), max_distance);
	return {mean_modelspace, max_distance};
}
	
AABB tz::physics::bound_aabb(const Mesh& mesh, const Matrix4x4& transform)
{
	constexpr float float_min = std::numeric_limits<float>::lowest();
	constexpr float float_max = std::numeric_limits<float>::max();
	float min_x = float_max, min_y = float_max, min_z = float_max;
	float max_x = float_min, max_y = float_min, max_z = float_min;
	auto positions_worldspace = mesh.get_positions();
	std::transform(positions_worldspace.begin(), positions_worldspace.end(), positions_worldspace.begin(), [&](const Vector3F& pos){return (transform * Vector4F{pos, 1.0f}).xyz();});
	for(const Vector3F& position_worldspace : positions_worldspace)
	{
		min_x = std::min(position_worldspace.x, min_x);
		min_y = std::min(position_worldspace.y, min_y);
		min_z = std::min(position_worldspace.z, min_z);
		max_x = std::max(position_worldspace.x, max_x);
		max_y = std::max(position_worldspace.y, max_y);
		max_z = std::max(position_worldspace.z, max_z);
	}
	return {Vector3F(min_x, min_y, min_z), Vector3F(max_x, max_y, max_z)};
}

std::optional<AABB> tz::physics::bound_aabb(const Model& model, const Matrix4x4& transform)
{
	if(model.get_number_of_meshes() == 0)
		return {};
	AABB box = tz::physics::bound_aabb(*model.get_mesh_by_id(0), transform);
	for(std::size_t i = 0; i < model.get_number_of_meshes(); i++)
	{
		box = box.expand_to(tz::physics::bound_aabb(*model.get_mesh_by_id(i), transform));
	}
	return {box};
}

std::optional<AABB> tz::physics::bound_aabb(const Asset& asset, const Matrix4x4& transform)
{
    if(asset.valid_mesh())
    {
        AABB mesh_box = tz::physics::bound_aabb(*asset.mesh, transform);
        if(asset.valid_model())
        {
            std::optional<AABB> box = tz::physics::bound_aabb(*asset.model, transform);
            if(box.has_value())
                mesh_box.expand_to(box.value());
        }
        return {mesh_box};
    }
    else if(asset.valid_model())
        return tz::physics::bound_aabb(*asset.model, transform);
    else
        return std::nullopt;
}

std::optional<AABB> tz::physics::bound_object(const StaticObject& object)
{
	return tz::physics::bound_aabb(object.get_asset(), object.transform.model());
}

std::ostream& operator<<(std::ostream& stream, const tz::physics::Axis2D& axis)
{
	using namespace tz::physics;
	switch(axis)
	{
		case Axis2D::X:
			stream << "X";
			break;
		case Axis2D::Y:
			stream << "Y";
			break;
		default:
			stream << "Not an Axis";
			break;
	}
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const tz::physics::Axis3D& axis)
{
	using namespace tz::physics;
	switch(axis)
	{
		case Axis3D::X:
			stream << "X";
			break;
		case Axis3D::Y:
			stream << "Y";
			break;
		case Axis3D::Z:
			stream << "Z";
			break;
		default:
			stream << "Not an Axis";
			break;
	}
	return stream;
}