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

	const Vector3F& get_centre() const;
	float get_radius() const;
	bool intersects(const BoundingSphere& rhs) const;
	virtual bool intersects(Boundary* other_boundary) const override;
private:
	const Vector3F centre;
	const float radius;
};

class AABB : public Boundary
{
public:
	AABB(Vector3F minimum, Vector3F maximum);
	AABB(const AABB& copy) = default;
	AABB(AABB&& move) = default;
	~AABB() = default;
	AABB& operator=(const AABB& rhs) = default;
	
	const Vector3F& get_minimum() const;
	const Vector3F& get_maximum() const;
	bool intersects(const AABB& rhs) const;
	bool intersects(const Vector3F& point) const;
	virtual bool intersects(Boundary* other_boundary) const override;
private:
	const Vector3F minimum, maximum;
};

class BoundingPlane : public Boundary
{
public:
	BoundingPlane(Vector3F normal, float distance);
	BoundingPlane(const BoundingPlane& copy) = default;
	BoundingPlane(BoundingPlane&& move) = default;
	~BoundingPlane() = default;
	BoundingPlane& operator=(const BoundingPlane& rhs) = default;

	const Vector3F& get_normal() const;
	float get_distance() const;
	BoundingPlane normalised() const;
	bool intersects(const BoundingSphere& other) const;
	virtual bool intersects(Boundary* other_boundary) const override;
private:
	const Vector3F normal;
	const float distance;
};

#endif