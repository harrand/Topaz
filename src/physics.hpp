#ifndef PHYSICS_HPP
#define PHYSICS_HPP
#include "boundary.hpp"
#include "object.hpp"

class Force
{
public:
	Force(Vector3F size = Vector3F());
	Force(const Force& copy) = default;
	Force(Force&& move) = default;
	~Force() = default;
	Force& operator=(const Force& rhs) = default;
	
	const Vector3F& getSize() const;
	Vector3F& getSizeR();
	Force operator+(const Force& other) const;
	Force operator-(const Force& other) const;
	Force operator*(float rhs) const;
	Force operator/(float rhs) const;
	Force& operator+=(const Force& other);
	Force& operator-=(const Force& other);
private:
	Vector3F size;
};

namespace tz::physics
{
	BoundingSphere boundSphere(const Object& object, const std::vector<std::unique_ptr<Mesh>>& all_meshes);
	AABB boundAABB(const Object& object, const std::vector<std::unique_ptr<Mesh>>& all_meshes);
}

#endif