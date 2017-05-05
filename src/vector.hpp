#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <cmath>
#include <vector>

template<typename T>
class Vector2
{
public:
	Vector2(T x = 0.0f, T y = 0.0f);
	Vector2(const Vector2& copy) = default;
	Vector2(Vector2&& move) = default;
	Vector2& operator=(const Vector2& rhs) = default;
	
	T getX() const;
	T getY() const;
	T& getXR();
	T& getYR();
	T length() const;
	T dot(Vector2 rhs) const;
	Vector2 normalised() const;
	
	Vector2 operator+(const Vector2& rhs) const;
	Vector2 operator-(const Vector2& rhs) const;
	Vector2 operator*(T scalar) const;
	bool operator<(const Vector2& rhs) const;
	bool operator>(const Vector2& rhs) const;
	bool operator==(const Vector2& rhs) const;
private:
	T x, y;
};

template<typename T>
class Vector3
{
public:
	Vector3(T x = 0.0f, T y = 0.0f, T z = 0.0f);
	Vector3(const Vector3& copy) = default;
	Vector3(Vector3&& move) = default;
	Vector3& operator=(const Vector3& rhs) = default;
	
	T getX() const;
	T getY() const;
	T getZ() const;
	T& getXR();
	T& getYR();
	T& getZR();
	T length() const;
	T dot(Vector3 rhs) const;
	Vector3 cross(Vector3 rhs) const;
	Vector3 normalised() const;
	
	Vector3 operator+(const Vector3& rhs) const;
	Vector3 operator-(const Vector3& rhs) const;
	Vector3 operator*(T scalar) const;
	Vector3 operator/(T scalar) const;
	bool operator<(const Vector3& rhs) const;
	bool operator>(const Vector3& rhs) const;
	bool operator==(const Vector3& rhs) const;
	void operator+=(const Vector3& rhs);
	void operator-=(const Vector3& rhs);
private:
	T x, y, z;
};

template<typename T>
class Vector4
{
public:
	Vector4(T x = 0.0f, T y = 0.0f, T z = 0.0f, T w = 0.0f);
	Vector4(const Vector4& copy) = default;
	Vector4(Vector4&& move) = default;
	Vector4& operator=(const Vector4& rhs) = default;
	
	T getX() const;
	T getY() const;
	T getZ() const;
	T getW() const;
	T& getXR();
	T& getYR();
	T& getZR();
	T& getWR();
	T length() const;
	T dot(Vector4 rhs) const;
	Vector4 cross(Vector4 rhs) const;
	Vector4 normalised() const;
	
	Vector4 operator+(const Vector4& rhs) const;
	Vector4 operator-(const Vector4& rhs) const;
	Vector4 operator*(T scalar) const;
	bool operator<(const Vector4& rhs) const;
	bool operator>(const Vector4& rhs) const;
	bool operator==(const Vector4& rhs) const;
private:
	T x, y, z, w;
};

typedef Vector2<float> Vector2F;
typedef Vector2<unsigned int> Vector2UI;
typedef Vector3<float> Vector3F;
typedef Vector3<unsigned int> Vector3UI;
typedef Vector4<float> Vector4F;
typedef Vector4<unsigned int> Vector4UI;
#endif