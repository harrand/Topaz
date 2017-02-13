#include "vector.hpp"

// Vector2F

Vector2F::Vector2F(float x, float y)
{
	this->x = x;
	this->y = y;
}

float Vector2F::getX()
{
	return this->x;
}

float Vector2F::getY()
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

float Vector2F::length()
{
	return sqrt((this->x * this->x) + (this->y * this->y));
}

float Vector2F::dot(Vector2F other)
{
	return (this->x * other.getX()) + (this->y * other.getY());
}

Vector2F Vector2F::normalised()
{
	return Vector2F(this->x / this->length(), this->y / this->length());
}

Vector2F Vector2F::operator+(Vector2F other)
{
	return Vector2F(this->x + other.getX(), this->y + other.getY());
}

Vector2F Vector2F::operator-(Vector2F other)
{
	return Vector2F(this->x - other.getX(), this->y - other.getY());
}

Vector2F Vector2F::operator*(float scalar)
{
	return Vector2F(this->x * scalar, this->y * scalar);
}

bool Vector2F::operator==(Vector2F other)
{
	return (this->x == other.getX()) && (this->y == other.getY());
}

// Vector3F

Vector3F::Vector3F(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

float Vector3F::getX()
{
	return this->x;
}

float Vector3F::getY()
{
	return this->y;
}

float Vector3F::getZ()
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

float Vector3F::length()
{
	return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
}

float Vector3F::dot(Vector3F other)
{
	return (this->x * other.getX()) + (this->y * other.getY()) + (this->z * other.getZ());
}

Vector3F Vector3F::cross(Vector3F other)
{
	return Vector3F((this->y * other.getZ()) - (this->z * other.getY()), (this->z * other.getX()) - (this->x * other.getZ()), (this->x * other.getY()) - (this->y * other.getX()));
}

Vector3F Vector3F::normalised()
{
	return Vector3F(this->x / this->length(), this->y / this->length(), this->z / this->length());
}

Vector3F Vector3F::operator+(Vector3F other)
{
	return Vector3F(this->x + other.getX(), this->y + other.getY(), this->z + other.getZ());
}

Vector3F Vector3F::operator-(Vector3F other)
{
	return Vector3F(this->x - other.getX(), this->y - other.getY(), this->z - other.getZ());
}

Vector3F Vector3F::operator*(float scalar)
{
	return Vector3F(this->x * scalar, this->y * scalar, this->z * scalar);
}

Vector3F Vector3F::operator/(float scalar)
{
	return Vector3F(this->x / scalar, this->y / scalar, this->z / scalar);
}

bool Vector3F::operator==(Vector3F other)
{
	return (this->x == other.getX()) && (this->y == other.getY()) && (this->z == other.getZ());
}

void Vector3F::operator+=(Vector3F other)
{
	(*this) = ((*this) + other);
}

void Vector3F::operator-=(Vector3F other)
{
	(*this) = ((*this) - other);
}

// Vector4F

Vector4F::Vector4F(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

float Vector4F::getX()
{
	return this->x;
}

float Vector4F::getY()
{
	return this->y;
}

float Vector4F::getZ()
{
	return this->z;
}

float Vector4F::getW()
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

float Vector4F::length()
{
	return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z) + (this->w * this->w));
}

float Vector4F::dot(Vector4F other)
{
	return (this->x * other.getX()) + (this->y * other.getY()) + (this->z * other.getZ() + (this->w * other.getW()));
}

Vector4F Vector4F::normalised()
{
	return Vector4F(this->x / this->length(), this->y / this->length(), this->z / this->length(), this->w / this->length());
}

Vector4F Vector4F::operator+(Vector4F other)
{
	return Vector4F(this->x + other.getX(), this->y + other.getY(), this->z + other.getZ(), this->w + other.getW());
}

Vector4F Vector4F::operator-(Vector4F other)
{
	return Vector4F(this->x - other.getX(), this->y - other.getY(), this->z - other.getZ(), this->w - other.getW());
}

Vector4F Vector4F::operator*(float scalar)
{
	return Vector4F(this->x * scalar, this->y * scalar, this->z * scalar, this->w * scalar);
}

bool Vector4F::operator==(Vector4F other)
{
	return (this->x == other.getX()) && (this->y == other.getY()) && (this->z == other.getZ()) && (this->w == other.getW());
}
