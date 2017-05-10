#include "utility.hpp"

Force::Force(Vector3F size): size(size){}

const Vector3F& Force::getSize() const
{
	return this->size;
}

void Force::setSize(Vector3F size)
{
	this->size = size;
}

Force Force::operator+(const Force& other) const
{
	return Force(this->size + other.getSize());
}

Force Force::operator-(const Force& other) const
{
	return Force(this->size - other.getSize());
}

Force Force::operator*(float rhs) const
{
	return Force(this->size * rhs);
}

Force Force::operator/(float rhs) const
{
	return Force(this->size / rhs);
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



