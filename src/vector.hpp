#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <cmath>
#include <vector>

class Vector2F
{
public:
	Vector2F(float x = 0.0f, float y = 0.0f);
	float getX() const;
	float getY() const;
	float& getXR();
	float& getYR();
	float length() const;
	float dot(Vector2F other) const;
	Vector2F normalised() const;
	
	Vector2F operator+(Vector2F other);
	Vector2F operator-(Vector2F other);
	Vector2F operator*(float scalar);
	bool operator==(Vector2F other);
private:
	float x, y;
};

class Vector3F
{
public:
	Vector3F(float x = 0.0f, float y = 0.0f, float z = 0.0f);
	float getX() const;
	float getY() const;
	float getZ() const;
	float& getXR();
	float& getYR();
	float& getZR();
	float length() const;
	float dot(Vector3F other) const;
	Vector3F cross(Vector3F other) const;
	Vector3F normalised() const;
	
	Vector3F operator+(Vector3F other);
	Vector3F operator-(Vector3F other);
	Vector3F operator*(float scalar);
	Vector3F operator/(float scalar);
	bool operator==(Vector3F other);
	void operator+=(Vector3F other);
	void operator-=(Vector3F other);
private:
	float x, y, z;
};

class Vector4F
{
public:
	Vector4F(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f);
	float getX() const;
	float getY() const;
	float getZ() const;
	float getW() const;
	float& getXR();
	float& getYR();
	float& getZR();
	float& getWR();
	float length() const;
	float dot(Vector4F other) const;
	Vector4F cross(Vector4F other) const;
	Vector4F normalised() const;
	
	Vector4F operator+(Vector4F other);
	Vector4F operator-(Vector4F other);
	Vector4F operator*(float scalar);
	bool operator==(Vector4F other);
private:
	float x, y, z, w;
};

#endif