#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <cmath>
#include <vector>

template<typename T>
class Vector2
{
public:
	Vector2<T>(T x = 0.0f, T y = 0.0f);
	Vector2<T>(const Vector2<T>& copy) = default;
	Vector2<T>(Vector2<T>&& move) = default;
	Vector2<T>& operator=(const Vector2<T>& rhs) = default;
	
	const T& getX() const;
	const T& getY() const;
	T& getXR();
	T& getYR();
	T length() const;
	T dot(Vector2<T> rhs) const;
	Vector2<T> normalised() const;
	
	Vector2<T> operator+(const Vector2<T>& rhs) const;
	Vector2<T> operator-(const Vector2<T>& rhs) const;
	Vector2<T> operator*(T scalar) const;
	bool operator<(const Vector2<T>& rhs) const;
	bool operator>(const Vector2<T>& rhs) const;
	bool operator==(const Vector2<T>& rhs) const;
private:
	T x, y;
};

template<typename T>
class Vector3
{
public:
	Vector3<T>(T x = 0.0f, T y = 0.0f, T z = 0.0f);
	Vector3<T>(const Vector3<T>& copy) = default;
	Vector3<T>(Vector3<T>&& move) = default;
	Vector3<T>& operator=(const Vector3<T>& rhs) = default;
	
	const T& getX() const;
	const T& getY() const;
	const T& getZ() const;
	T& getXR();
	T& getYR();
	T& getZR();
	T length() const;
	T dot(Vector3<T> rhs) const;
	Vector3 cross(Vector3<T> rhs) const;
	Vector3 normalised() const;
	
	Vector3<T> operator+(const Vector3<T>& rhs) const;
	Vector3<T> operator-(const Vector3<T>& rhs) const;
	Vector3<T> operator*(T scalar) const;
	Vector3<T> operator/(T scalar) const;
	bool operator<(const Vector3<T>& rhs) const;
	bool operator>(const Vector3<T>& rhs) const;
	bool operator==(const Vector3<T>& rhs) const;
	Vector3<T>& operator+=(const Vector3<T>& rhs);
	Vector3<T>& operator-=(const Vector3<T>& rhs);
private:
	T x, y, z;
};

template<typename T>
class Vector4
{
public:
	Vector4<T>(T x = 0.0f, T y = 0.0f, T z = 0.0f, T w = 0.0f);
	Vector4<T>(const Vector4& copy) = default;
	Vector4<T>(Vector4&& move) = default;
	Vector4<T>& operator=(const Vector4<T>& rhs) = default;
	
	const T& getX() const;
	const T& getY() const;
	const T& getZ() const;
	const T& getW() const;
	T& getXR();
	T& getYR();
	T& getZR();
	T& getWR();
	T length() const;
	T dot(Vector4<T> rhs) const;
	Vector4<T> cross(Vector4<T> rhs) const;
	Vector4<T> normalised() const;
	
	Vector4<T> operator+(const Vector4<T>& rhs) const;
	Vector4<T> operator-(const Vector4<T>& rhs) const;
	Vector4<T> operator*(T scalar) const;
	bool operator<(const Vector4<T>& rhs) const;
	bool operator>(const Vector4<T>& rhs) const;
	bool operator==(const Vector4<T>& rhs) const;
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