#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <cmath>
#include <vector>

class Vector2F
{
public:
	Vector2F(float x = 0.0f, float y = 0.0f);
	float getX();
	float getY();
	float& getXR();
	float& getYR();
	float length();
	float dot(Vector2F other);
	Vector2F normalised();
	
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
	float getX();
	float getY();
	float getZ();
	float& getXR();
	float& getYR();
	float& getZR();
	float length();
	float dot(Vector3F other);
	Vector3F cross(Vector3F other);
	Vector3F normalised();
	
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
	float getX();
	float getY();
	float getZ();
	float getW();
	float& getXR();
	float& getYR();
	float& getZR();
	float& getWR();
	float length();
	float dot(Vector4F other);
	Vector4F cross(Vector4F other);
	Vector4F normalised();
	
	Vector4F operator+(Vector4F other);
	Vector4F operator-(Vector4F other);
	Vector4F operator*(float scalar);
	bool operator==(Vector4F other);
private:
	float x, y, z, w;
};

#endif