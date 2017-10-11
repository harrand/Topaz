#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <cmath>
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
	Vector2<T>(T x = T(), T y = T()): x(x), y(y){}
	constexpr Vector2<T>(const std::array<T, 2>& data): x(data[0]), y(data[1]){}
	inline Vector2<T>(const Vector2<T>& copy) = default;
	inline Vector2<T>(Vector2<T>&& move) = default;
	inline ~Vector2<T>() = default;
	inline Vector2<T>& operator=(const Vector2<T>& rhs) = default;
	
	inline Vector2POD to_raw() const
	{
		Vector2POD raw;
		raw.x = this->get_x();
		raw.y = this->get_y();
		return raw;
	}
	
	inline const T& get_x() const
	{
		return this->x;
	}
	
	inline const T& get_y() const
	{
		return this->y;
	}
	
	inline void set_x(T x)
	{
		this->x = x;
	}
	
	inline void set_y(T y)
	{
		this->y = y;
	}
	/*
	inline T& get_x_r()
	{
		return this->x;
	}
	
	inline T& get_y_r()
	{
		return this->y;
	}
	*/
	
	inline T length() const
	{
		return sqrt((this->x * this->x) + (this->y * this->y));
	}
	
	inline T dot(const Vector2<T>& rhs) const
	{
		return (this->x * rhs.get_x()) + (this->y * rhs.get_y());
	}
	
	inline Vector2<T> normalised() const
	{
		return {this->x / this->length(), this->y / this->length()};
	}
	
	inline Vector2<T> operator+(const Vector2<T>& rhs) const
	{
		return {this->x + rhs.get_x(), this->y + rhs.get_y()};
	}
	
	inline Vector2<T> operator-(const Vector2<T>& rhs) const
	{
		return {this->x - rhs.get_x(), this->y - rhs.get_y()};
	}
	
	inline Vector2<T> operator*(T scalar) const
	{
		return {this->x * scalar, this->y * scalar};
	}
	
	inline Vector2<T> operator/(T scalar) const
	{
		return {this->x / scalar, this->y / scalar};
	}
	
	inline bool operator<(const Vector2<T>& rhs) const
	{
		return this->length() < rhs.length();
	}
	
	inline bool operator>(const Vector2<T>& rhs) const
	{
		return this->length() > rhs.length();
	}
	
	inline bool operator==(const Vector2<T>& rhs) const
	{
		return (this->x == rhs.get_x()) && (this->y == rhs.get_y());
	}
	
	inline Vector2<T>& operator+=(const Vector2<T>& rhs)
	{
		(*this) = (*this) + rhs;
		return (*this);
	}
	
	inline Vector2<T>& operator-=(const Vector2<T>& rhs)
	{
		(*this) = (*this) - rhs;
		return (*this);
	}
protected:
	T x, y;
};

template<typename T>
class Vector3: public Vector2<T>
{
private:
	using Vector2<T>::normalised;
public:
	inline Vector3<T>(T x = T(), T y = T(), T z = T()): Vector2<T>(x, y), z(z){}
	constexpr Vector3<T>(const std::array<T, 3>& data): Vector2<T>(std::array<T, 2>({data[0], data[1]})), z(data[2]){}
	inline Vector3<T>(const Vector3<T>& copy) = default;
	inline Vector3<T>(Vector3<T>&& move) = default;
	inline ~Vector3<T>() = default;
	inline Vector3<T>& operator=(const Vector3<T>& rhs) = default;
	
	inline Vector3POD to_raw() const
	{
		Vector3POD raw;
		raw.x = this->get_x();
		raw.y = this->get_y();
		raw.z = this->get_z();
		return raw;
	}
	
	inline const T& get_z() const
	{
		return this->z;
	}
	
	inline void set_z(T z)
	{
		this->z = z;
	}
	
	inline T length() const 
	{
		return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
	}
	
	inline T dot(const Vector3<T>& rhs) const
	{
		return (this->x * rhs.get_x()) + (this->y * rhs.get_y()) + (this->z * rhs.get_z());
	}
	
	inline Vector3<T> cross(const Vector3<T>& rhs) const
	{
		return {(this->y * rhs.get_z()) - (this->z * rhs.get_y()), (this->z * rhs.get_x()) - (this->x * rhs.get_z()), (this->x * rhs.get_y()) - (this->y * rhs.get_x())};
	}
	
	inline Vector3<T> normalised() const
	{
		return {this->x / this->length(), this->y / this->length(), this->z / this->length()};
	}
	
	inline Vector3<T> operator+(const Vector3<T>& rhs) const
	{
		return {this->x + rhs.get_x(), this->y + rhs.get_y(), this->z + rhs.get_z()};
	}
	
	inline Vector3<T> operator-(const Vector3<T>& rhs) const
	{
		return {this->x - rhs.get_x(), this->y - rhs.get_y(), this->z - rhs.get_z()};
	}
	
	inline Vector3<T> operator*(T scalar) const
	{
		return {this->x * scalar, this->y * scalar, this->z * scalar};
	}
	
	inline Vector3<T> operator/(T scalar) const
	{
		return {this->x / scalar, this->y / scalar, this->z / scalar};
	}
	
	inline bool operator<(const Vector3<T>& rhs) const
	{
		return this->length() < rhs.length();
	}
	
	inline bool operator>(const Vector3<T>& rhs) const
	{
		return this->length() > rhs.length();
	}
	
	inline bool operator<=(const Vector3<T>& rhs) const
	{
		return *this < rhs || *this == rhs;
	}
	
	inline bool operator>=(const Vector3<T>& rhs) const
	{
		return *this > rhs || *this == rhs;
	}
	
	inline bool operator==(const Vector3<T>& rhs) const
	{
		return (this->x == rhs.get_x()) && (this->y == rhs.get_y()) && (this->z == rhs.get_z());
	}
	
	inline Vector3<T>& operator+=(const Vector3<T>& rhs)
	{
		(*this) = ((*this) + rhs);
		return *this;
	}
	
	inline Vector3<T>& operator-=(const Vector3<T>& rhs)
	{
		(*this) = ((*this) - rhs);
		return *this;
	}
protected:
	T z;
};

template<typename T>
class Vector4: public Vector3<T>
{
private:
	using Vector3<T>::normalised;
public:
	inline Vector4<T>(T x = T(), T y = T(), T z = T(), T w = T()): Vector3<T>(x, y, z), w(w){}
	constexpr Vector4<T>(const std::array<T, 4>& data): Vector3<T>(std::array<T, 3>({data[0], data[1], data[2]})), w(data[3]){}
	inline Vector4<T>(const Vector4& copy) = default;
	inline Vector4<T>(Vector4&& move) = default;
	inline ~Vector4<T>() = default;
	inline Vector4<T>& operator=(const Vector4<T>& rhs) = default;
	
	inline Vector4POD to_raw() const
	{
		Vector4POD raw;
		raw.x = this->get_x();
		raw.y = this->get_y();
		raw.z = this->get_z();
		raw.w = this->get_w();
		return raw;
	}
	
	inline const T& get_w() const
	{
		return this->w;
	}
	
	void set_w(T w)
	{
		this->w = w;
	}
	
	inline T length() const
	{
		return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z) + (this->w * this->w));
	}
	
	inline T dot(Vector4<T> rhs) const
	{
		return (this->x * rhs.get_x()) + (this->y * rhs.get_y()) + (this->z * rhs.get_z() + (this->w * rhs.get_w()));
	}
	
	inline Vector4<T> normalised() const
	{
		return {this->x / this->length(), this->y / this->length(), this->z / this->length(), this->w / this->length()};
	}
	
	inline Vector4<T> operator+(const Vector4<T>& rhs) const
	{
		return {this->x + rhs.get_x(), this->y + rhs.get_y(), this->z + rhs.get_z(), this->w + rhs.get_w()};
	}
	
	inline Vector4<T> operator-(const Vector4<T>& rhs) const
	{
		return {this->x - rhs.get_x(), this->y - rhs.get_y(), this->z - rhs.get_z(), this->w - rhs.get_w()};
	}
	
	inline Vector4<T> operator*(T scalar) const
	{
		return {this->x * scalar, this->y * scalar, this->z * scalar, this->w * scalar};
	}
	
	inline Vector4<T> operator/(T scalar) const
	{
		return {this->x / scalar, this->y / scalar, this->z / scalar, this->w / scalar};
	}
	
	inline bool operator<(const Vector4<T>& rhs) const
	{
		return this->length() < rhs.length();
	}
	
	inline bool operator>(const Vector4<T>& rhs) const
	{
		return this->length() > rhs.length();
	}
	
	inline bool operator==(const Vector4<T>& rhs) const
	{
		return (this->x == rhs.get_x()) && (this->y == rhs.get_y()) && (this->z == rhs.get_z()) && (this->w == rhs.get_w());
	}
	
	inline Vector4<T>& operator+=(const Vector4<T>& rhs)
	{
		(*this) = (*this) + rhs;
		return (*this);
	}
	
	inline Vector4<T>& operator-=(const Vector4<T>& rhs)
	{
		(*this) = (*this) - rhs;
		return (*this);
	}
protected:
	T w;
};
using Vector2F = Vector2<float>;
using Vector3F = Vector3<float>;
using Vector4F = Vector4<float>;
#endif