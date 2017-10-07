#ifndef BOUNDARY_HPP
#define BOUNDARY_HPP
#include "vector.hpp"

class Boundary
{
public:
	Boundary() = default;
	Boundary(const Boundary& copy) = default;
	Boundary(Boundary&& move) = default;
	~Boundary() = default;
	Boundary& operator=(const Boundary& rhs) = default;
	
	virtual bool intersects(Boundary* other_boundary) const = 0;
};

class BoundingSphere : public Boundary
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
	virtual bool intersects(Boundary* other_boundary) const;
private:
	Vector3F centre;
	float radius;
};

class AABB : public Boundary
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
	bool intersects(const Vector3F& point) const;
	bool intersects(Boundary* other_boundary) const;
private:
	Vector3F minimum, maximum;
};

class BoundingPlane : public Boundary
{
public:
	BoundingPlane(Vector3F normal, float distance);
	BoundingPlane(const BoundingPlane& copy) = default;
	BoundingPlane(BoundingPlane&& move) = default;
	~BoundingPlane() = default;
	BoundingPlane& operator=(const BoundingPlane& rhs) = default;

	const Vector3F& getNormal() const;
	Vector3F& getNormalR();
	float getDistance() const;
	float& getDistanceR();
	BoundingPlane normalised() const;
	bool intersects(const BoundingSphere& other) const;
	bool intersects(Boundary* other_boundary) const;
private:
	Vector3F normal;
	float distance;
};

#endif