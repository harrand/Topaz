#ifndef PHYSICS_HPP
#define PHYSICS_HPP
#include "boundary.hpp"
#include "object.hpp"

namespace tz::physics
{
	inline BoundingSphere boundSphere(const Object& object, const std::vector<std::unique_ptr<Mesh>>& all_meshes)
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
		return BoundingSphere(mean, *std::max_element(distances.begin(), distances.end()));
	}
}

#endif