#include "physics.hpp"
#include <limits>

Force::Force(Vector3F size): size(std::move(size)){}

Force Force::operator+(const Force& other) const
{
	return (this->size + other.size);
}

Force Force::operator-(const Force& other) const
{
	return (this->size - other.size);
}

Force Force::operator*(float rhs) const
{
	return (this->size * rhs);
}

Force Force::operator/(float rhs) const
{
	return (this->size / rhs);
}

Force& Force::operator+=(const Force& other)
{
	this->size += other.size;
	return *this;
}

Force& Force::operator-=(const Force& other)
{
	this->size -= other.size;
	this->size -= other.size;
	return *this;
}

bool Force::operator==(const Force& rhs) const
{
	return this->size == rhs.size;
}

BoundingSphere tz::physics::bound_sphere(const Object3D& object)
{
	// get list of positions in world space
	std::vector<Vector3F> positions_worldspace;
	const std::vector<Vector3F>& positions_modelspace = object.get_mesh().get_positions();
	positions_worldspace.reserve(positions_modelspace.size());
	for(Vector3F position_modelspace : positions_modelspace)
	{
		Vector4F position_modelspace_homogeneous(position_modelspace.x, position_modelspace.y, position_modelspace.z, 1.0f);
		Vector4F position_worldspace_homogeneous = tz::transform::model(object.position, object.rotation, object.scale) * position_modelspace_homogeneous;
		positions_worldspace.emplace_back(position_worldspace_homogeneous.x, position_worldspace_homogeneous.y, position_worldspace_homogeneous.z);
	}
	// Objects are static so don't need to worry about updating this ever again. Best solution is O(n) which is to find the average position and use that as centre, and then find maximum distance and make that the distance
	Vector3F mean = std::accumulate(positions_worldspace.begin(), positions_worldspace.end(), Vector3F(), [](const Vector3F& a, const Vector3F& b) -> Vector3F{return a + b;}) / positions_worldspace.size();
	// get mean of the positions_worldspace to be the centre
	// if centre is the mean, then distance is the distance between the mean and whatever is furthest away from the mean i.e biggest outlier
	std::transform(positions_worldspace.begin(), positions_worldspace.end(), positions_worldspace.begin(), [&mean](const Vector3F& position) -> Vector3F{return mean - position;});
	std::vector<float> distances;
	distances.reserve(positions_worldspace.size());
	for(auto displacement : positions_worldspace)
		distances.push_back(std::fabs(displacement.length()));
	return {mean, *std::max_element(distances.begin(), distances.end())};
}
	
AABB tz::physics::bound_aabb(const Object3D& object)
{
	// once again get positions in worldspace
	const std::vector<Vector3F>& positions_modelspace = object.get_mesh().get_positions();
	std::vector<Vector3F> positions_worldspace;
	positions_worldspace.reserve(positions_modelspace.size());
	for(Vector3F position_modelspace : positions_modelspace)
	{
		Vector4F position_modelspace_homogeneous(position_modelspace.x, position_modelspace.y, position_modelspace.z, 1.0f);
		Vector4F position_worldspace_homogeneous = tz::transform::model(object.position, object.rotation, object.scale) * position_modelspace_homogeneous;
		positions_worldspace.emplace_back(position_worldspace_homogeneous.x, position_worldspace_homogeneous.y, position_worldspace_homogeneous.z);
	}
	constexpr float float_min = std::numeric_limits<float>::lowest();
	constexpr float float_max = std::numeric_limits<float>::max();
	float min_x = float_max, min_y = float_max, min_z = float_max;
	float max_x = float_min, max_y = float_min, max_z = float_min;
	for(const Vector3F& position_worldspace : positions_worldspace)
	{
		min_x = std::min(position_worldspace.x, min_x);
		min_y = std::min(position_worldspace.y, min_y);
		min_z = std::min(position_worldspace.z, min_z);
		max_x = std::max(position_worldspace.x, max_x);
		max_y = std::max(position_worldspace.y, max_y);
		max_z = std::max(position_worldspace.z, max_z);
	}
	// get minimum and maximum values for all three dimensions. use the minimums and maximums to construct the AABB
	return {Vector3F(min_x, min_y, min_z), Vector3F(max_x, max_y, max_z)};
}