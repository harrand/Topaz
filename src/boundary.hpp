#ifndef BOUNDARY_HPP
#define BOUNDARY_HPP
#include "vector.hpp"

class BoundingSphere
{
public:
	BoundingSphere(Vector3F centre, float radius);
	BoundingSphere(const BoundingSphere& copy) = default;
	BoundingSphere(BoundingSphere&& move) = default;
	~BoundingSphere() = default;
	BoundingSphere& operator=(const BoundingSphere& rhs) = default;

	const Vector3F& getCentre() const;
	Vector3F& getCentreR();
	float getRadius() const;
	float& getRadiusR();
	bool intersects(const BoundingSphere& rhs) const;
private:
	Vector3F centre;
	float radius;
};

class AABB
{
public:
	AABB(Vector3F minimum, Vector3F maximum);
	AABB(const AABB& copy) = default;
	AABB(AABB&& move) = default;
	~AABB() = default;
	AABB& operator=(const AABB& rhs) = default;
	
	const Vector3F& getMinimum() const;
	Vector3F& getMinimumR();
	const Vector3F& getMaximum() const;
	Vector3F& getMaximumR();
	bool intersects(const AABB& rhs) const;
private:
	Vector3F minimum, maximum;
};

class BoundingPlane
{
public:
	BoundingPlane(Vector3F normal, float distance);
	
	const Vector3F& getNormal() const;
	Vector3F& getNormalR();
	float getDistance() const;
	float& getDistanceR();
	BoundingPlane normalised() const;
	bool intersects(const BoundingSphere& other) const;
private:
	Vector3F normal;
	float distance;
};

#endif