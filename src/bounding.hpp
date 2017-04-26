#ifndef BOUNDING_HPP
#define BOUNDING_HPP
#include "vector.hpp"

class Boundary
{
public:
	Boundary() = default;
	Boundary(const Boundary& copy) = default;
	Boundary(Boundary&& move) = default;
	Boundary& operator=(const Boundary& rhs) = default;
	
	virtual bool intersects(const Vector3F& point) const = 0;
};

class AABB: public Boundary
{
public:
	AABB(Vector3F x = Vector3F(), Vector3F y = Vector3F());
	virtual bool intersects(const Vector3F& point) const;
	const Vector3F& getMinimum() const;
	const Vector3F& getMaximum() const;
	Vector3F& getMinimumR();
	Vector3F getMaximumR();
private:
	Vector3F min, max;
};

#endif