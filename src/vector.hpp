#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <array>

// C-style POD structs so that trivial structs can be passed to OpenGL buffers as the memory is guaranteed to be contiguous
struct Vector2POD
{
	float x, y;
};

struct Vector3POD
{
	float x, y, z;
};

struct Vector4POD
{
	float x, y, z, w;
};

template<typename T>
class Vector2
{
public:
	Vector2<T>(T x = T(), T y = T());
	constexpr Vector2<T>(const std::array<T, 2>& data);
	Vector2<T>(const Vector2<T>& copy) = default;
	Vector2<T>(Vector2<T>&& move) = default;
	~Vector2<T>() = default;
	Vector2<T>& operator=(const Vector2<T>& rhs) = default;
	Vector2POD to_raw() const;
	const T& get_x() const;
	const T& get_y() const;
	void set_x(T x);
	void set_y(T y);
	T length() const;
	T dot(const Vector2<T>& rhs) const;
	Vector2<T> normalised() const;
	Vector2<T> operator+(const Vector2<T>& rhs) const;
	Vector2<T> operator-(const Vector2<T>& rhs) const;
	Vector2<T> operator*(T scalar) const;
	Vector2<T> operator/(T scalar) const;
	bool operator<(const Vector2<T>& rhs) const;
	bool operator>(const Vector2<T>& rhs) const;
	bool operator==(const Vector2<T>& rhs) const;
	Vector2<T>& operator+=(const Vector2<T>& rhs);
	Vector2<T>& operator-=(const Vector2<T>& rhs);
protected:
	T x, y;
};

template<typename T>
class Vector3: public Vector2<T>
{
private:
	using Vector2<T>::normalised;
public:
	Vector3<T>(T x = T(), T y = T(), T z = T());
	constexpr Vector3<T>(const std::array<T, 3>& data);
	Vector3<T>(const Vector3<T>& copy) = default;
	Vector3<T>(Vector3<T>&& move) = default;
	~Vector3<T>() = default;
	Vector3<T>& operator=(const Vector3<T>& rhs) = default;
	
	Vector3POD to_raw() const;
	const T& get_z() const;
	void set_z(T z);
	T length() const;
	T dot(const Vector3<T>& rhs) const;
	Vector3<T> cross(const Vector3<T>& rhs) const;
	Vector3<T> normalised() const;
	Vector3<T> operator+(const Vector3<T>& rhs) const;
	Vector3<T> operator-(const Vector3<T>& rhs) const;
	Vector3<T> operator*(T scalar) const;
	Vector3<T> operator/(T scalar) const;
	bool operator<(const Vector3<T>& rhs) const;
	bool operator>(const Vector3<T>& rhs) const;
	bool operator<=(const Vector3<T>& rhs) const;
	bool operator>=(const Vector3<T>& rhs) const;
	bool operator==(const Vector3<T>& rhs) const;
	Vector3<T>& operator+=(const Vector3<T>& rhs);
	Vector3<T>& operator-=(const Vector3<T>& rhs);
protected:
	T z;
};

template<typename T>
class Vector4: public Vector3<T>
{
private:
	using Vector3<T>::normalised;
public:
	Vector4<T>(T x = T(), T y = T(), T z = T(), T w = T());
	constexpr Vector4<T>(const std::array<T, 4>& data);
	Vector4<T>(const Vector4& copy) = default;
	Vector4<T>(Vector4&& move) = default;
	~Vector4<T>() = default;
	Vector4<T>& operator=(const Vector4<T>& rhs) = default;
	
	Vector4POD to_raw() const;
	const T& get_w() const;
	void set_w(T w);
	T length() const;
	T dot(Vector4<T> rhs) const;
	Vector4<T> normalised() const;
	Vector4<T> operator+(const Vector4<T>& rhs) const;
	Vector4<T> operator-(const Vector4<T>& rhs) const;
	Vector4<T> operator*(T scalar) const;
	Vector4<T> operator/(T scalar) const;
	bool operator<(const Vector4<T>& rhs) const;
	bool operator>(const Vector4<T>& rhs) const;
	bool operator==(const Vector4<T>& rhs) const;
	Vector4<T>& operator+=(const Vector4<T>& rhs);
	Vector4<T>& operator-=(const Vector4<T>& rhs);
protected:
	T w;
};

#include "vector.inl"
using Vector2F = Vector2<float>;
using Vector3F = Vector3<float>;
using Vector4F = Vector4<float>;
#endif