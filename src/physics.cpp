#include "physics.hpp"
#include <limits>

Force::Force(Vector3F size): size(std::move(size)){}

const Vector3F& Force::getSize() const
{
	return this->size;
}

Vector3F& Force::getSizeR()
{
	return this->size;
}

Force Force::operator+(const Force& other) const
{
	return (this->size + other.getSize());
}

Force Force::operator-(const Force& other) const
{
	return (this->size - other.getSize());
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
	this->size += other.getSize();
	return *this;
}

Force& Force::operator-=(const Force& other)
{
	this->size -= other.getSize();
	return *this;
}