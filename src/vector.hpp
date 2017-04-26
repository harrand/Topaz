#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <cmath>
#include <vector>

class Vector2F
{
public:
	Vector2F(float x = 0.0f, float y = 0.0f);
	Vector2F(const Vector2F& copy) = default;
	Vector2F(Vector2F&& move) = default;
	Vector2F& operator=(const Vector2F& rhs) = default;
	
	float getX() const;
	float getY() const;
	float& getXR();
	float& getYR();
	float length() const;
	float dot(Vector2F rhs) const;
	Vector2F normalised() const;
	
	Vector2F operator+(const Vector2F& rhs) const;
	Vector2F operator-(const Vector2F& rhs) const;
	Vector2F operator*(float scalar) const;
	bool operator<(const Vector2F& rhs) const;
	bool operator>(const Vector2F& rhs) const;
	bool operator==(const Vector2F& rhs) const;
private:
	float x, y;
};

class Vector3F
{
public:
	Vector3F(float x = 0.0f, float y = 0.0f, float z = 0.0f);
	Vector3F(const Vector3F& copy) = default;
	Vector3F(Vector3F&& move) = default;
	Vector3F& operator=(const Vector3F& rhs) = default;
	
	float getX() const;
	float getY() const;
	float getZ() const;
	float& getXR();
	float& getYR();
	float& getZR();
	float length() const;
	float dot(Vector3F rhs) const;
	Vector3F cross(Vector3F rhs) const;
	Vector3F normalised() const;
	
	Vector3F operator+(const Vector3F& rhs) const;
	Vector3F operator-(const Vector3F& rhs) const;
	Vector3F operator*(float scalar) const;
	Vector3F operator/(float scalar) const;
	bool operator<(const Vector3F& rhs) const;
	bool operator>(const Vector3F& rhs) const;
	bool operator==(const Vector3F& rhs) const;
	void operator+=(const Vector3F& rhs);
	void operator-=(const Vector3F& rhs);
private:
	float x, y, z;
};

class Vector4F
{
public:
	Vector4F(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f);
	Vector4F(const Vector4F& copy) = default;
	Vector4F(Vector4F&& move) = default;
	Vector4F& operator=(const Vector4F& rhs) = default;
	
	float getX() const;
	float getY() const;
	float getZ() const;
	float getW() const;
	float& getXR();
	float& getYR();
	float& getZR();
	float& getWR();
	float length() const;
	float dot(Vector4F rhs) const;
	Vector4F cross(Vector4F rhs) const;
	Vector4F normalised() const;
	
	Vector4F operator+(const Vector4F& rhs) const;
	Vector4F operator-(const Vector4F& rhs) const;
	Vector4F operator*(float scalar) const;
	bool operator<(const Vector4F& rhs) const;
	bool operator>(const Vector4F& rhs) const;
	bool operator==(const Vector4F& rhs) const;
private:
	float x, y, z, w;
};

#endif