#include "vector.hpp"

// Vector2F

Vector2F::Vector2F(float x, float y)
{
	this->x = x;
	this->y = y;
}

float Vector2F::getX() const
{
	return this->x;
}

float Vector2F::getY() const
{
	return this->y;
}

float& Vector2F::getXR()
{
	return this->x;
}

float& Vector2F::getYR()
{
	return this->y;
}

float Vector2F::length() const
{
	return sqrt((this->x * this->x) + (this->y * this->y));
}

float Vector2F::dot(Vector2F rhs) const
{
	return (this->x * rhs.getX()) + (this->y * rhs.getY());
}

Vector2F Vector2F::normalised() const
{
	return Vector2F(this->x / this->length(), this->y / this->length());
}

Vector2F Vector2F::operator+(const Vector2F& rhs) const
{
	return Vector2F(this->x + rhs.getX(), this->y + rhs.getY());
}

Vector2F Vector2F::operator-(const Vector2F& rhs) const
{
	return Vector2F(this->x - rhs.getX(), this->y - rhs.getY());
}

Vector2F Vector2F::operator*(float scalar) const
{
	return Vector2F(this->x * scalar, this->y * scalar);
}

bool Vector2F::operator<(const Vector2F& rhs) const
{
	return this->length() < rhs.length();
}

bool Vector2F::operator>(const Vector2F& rhs) const
{
	return this->length() > rhs.length();
}

bool Vector2F::operator==(const Vector2F& rhs) const
{
	return (this->x == rhs.getX()) && (this->y == rhs.getY());
}

// Vector3F

Vector3F::Vector3F(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

float Vector3F::getX() const
{
	return this->x;
}

float Vector3F::getY() const
{
	return this->y;
}

float Vector3F::getZ() const
{
	return this->z;
}

float& Vector3F::getXR()
{
	return this->x;
}

float& Vector3F::getYR()
{
	return this->y;
}

float& Vector3F::getZR()
{
	return this->z;
}

float Vector3F::length() const
{
	return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
}

float Vector3F::dot(Vector3F rhs) const
{
	return (this->x * rhs.getX()) + (this->y * rhs.getY()) + (this->z * rhs.getZ());
}

Vector3F Vector3F::cross(Vector3F rhs) const
{
	return Vector3F((this->y * rhs.getZ()) - (this->z * rhs.getY()), (this->z * rhs.getX()) - (this->x * rhs.getZ()), (this->x * rhs.getY()) - (this->y * rhs.getX()));
}

Vector3F Vector3F::normalised() const
{
	return Vector3F(this->x / this->length(), this->y / this->length(), this->z / this->length());
}

Vector3F Vector3F::operator+(const Vector3F& rhs) const
{
	return Vector3F(this->x + rhs.getX(), this->y + rhs.getY(), this->z + rhs.getZ());
}

Vector3F Vector3F::operator-(const Vector3F& rhs) const
{
	return Vector3F(this->x - rhs.getX(), this->y - rhs.getY(), this->z - rhs.getZ());
}

Vector3F Vector3F::operator*(float scalar) const
{
	return Vector3F(this->x * scalar, this->y * scalar, this->z * scalar);
}

Vector3F Vector3F::operator/(float scalar) const
{
	return Vector3F(this->x / scalar, this->y / scalar, this->z / scalar);
}

bool Vector3F::operator<(const Vector3F& rhs) const
{
	return this->length() < rhs.length();
}

bool Vector3F::operator>(const Vector3F& rhs) const
{
	return this->length() > rhs.length();
}

bool Vector3F::operator==(const Vector3F& rhs) const
{
	return (this->x == rhs.getX()) && (this->y == rhs.getY()) && (this->z == rhs.getZ());
}

void Vector3F::operator+=(const Vector3F& rhs)
{
	(*this) = ((*this) + rhs);
}

void Vector3F::operator-=(const Vector3F& rhs)
{
	(*this) = ((*this) - rhs);
}

// Vector4F

Vector4F::Vector4F(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

float Vector4F::getX() const
{
	return this->x;
}

float Vector4F::getY() const
{
	return this->y;
}

float Vector4F::getZ() const
{
	return this->z;
}

float Vector4F::getW() const
{
	return this->w;
}

float& Vector4F::getXR()
{
	return this->x;
}

float& Vector4F::getYR()
{
	return this->y;
}

float& Vector4F::getZR()
{
	return this->z;
}

float& Vector4F::getWR()
{
	return this->w;
}

float Vector4F::length() const
{
	return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z) + (this->w * this->w));
}

float Vector4F::dot(Vector4F rhs) const
{
	return (this->x * rhs.getX()) + (this->y * rhs.getY()) + (this->z * rhs.getZ() + (this->w * rhs.getW()));
}

Vector4F Vector4F::normalised() const
{
	return Vector4F(this->x / this->length(), this->y / this->length(), this->z / this->length(), this->w / this->length());
}

Vector4F Vector4F::operator+(const Vector4F& rhs) const
{
	return Vector4F(this->x + rhs.getX(), this->y + rhs.getY(), this->z + rhs.getZ(), this->w + rhs.getW());
}

Vector4F Vector4F::operator-(const Vector4F& rhs) const
{
	return Vector4F(this->x - rhs.getX(), this->y - rhs.getY(), this->z - rhs.getZ(), this->w - rhs.getW());
}

Vector4F Vector4F::operator*(float scalar) const
{
	return Vector4F(this->x * scalar, this->y * scalar, this->z * scalar, this->w * scalar);
}

bool Vector4F::operator<(const Vector4F& rhs) const
{
	return this->length() < rhs.length();
}

bool Vector4F::operator>(const Vector4F& rhs) const
{
	return this->length() > rhs.length();
}

bool Vector4F::operator==(const Vector4F& rhs) const
{
	return (this->x == rhs.getX()) && (this->y == rhs.getY()) && (this->z == rhs.getZ()) && (this->w == rhs.getW());
}
