#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <cmath>
#include <array>

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
	
	inline Vector2POD toRaw() const
	{
		Vector2POD raw;
		raw.x = this->getX();
		raw.y = this->getY();
		return raw;
	}
	
	inline const T& getX() const
	{
		return this->x;
	}
	
	inline const T& getY() const
	{
		return this->y;
	}
	
	inline T& getXR()
	{
		return this->x;
	}
	
	inline T& getYR()
	{
		return this->y;
	}
	
	inline T length() const
	{
		return sqrt((this->x * this->x) + (this->y * this->y));
	}
	
	inline T dot(const Vector2<T>& rhs) const
	{
		return (this->x * rhs.getX()) + (this->y * rhs.getY());
	}
	
	inline Vector2<T> normalised() const
	{
		return {this->x / this->length(), this->y / this->length()};
	}
	
	inline Vector2<T> operator+(const Vector2<T>& rhs) const
	{
		return {this->x + rhs.getX(), this->y + rhs.getY()};
	}
	
	inline Vector2<T> operator-(const Vector2<T>& rhs) const
	{
		return {this->x - rhs.getX(), this->y - rhs.getY()};
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
		return (this->x == rhs.getX()) && (this->y == rhs.getY());
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
	
	inline Vector3POD toRaw() const
	{
		Vector3POD raw;
		raw.x = this->getX();
		raw.y = this->getY();
		raw.z = this->getZ();
		return raw;
	}
	
	inline const T& getZ() const
	{
		return this->z;
	}
	
	inline T& getZR()
	{
		return this->z;
	}
	
	inline T length() const 
	{
		return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
	}
	
	inline T dot(const Vector3<T>& rhs) const
	{
		return (this->x * rhs.getX()) + (this->y * rhs.getY()) + (this->z * rhs.getZ());
	}
	
	inline Vector3<T> cross(const Vector3<T>& rhs) const
	{
		return {(this->y * rhs.getZ()) - (this->z * rhs.getY()), (this->z * rhs.getX()) - (this->x * rhs.getZ()), (this->x * rhs.getY()) - (this->y * rhs.getX())};
	}
	
	inline Vector3<T> normalised() const
	{
		return {this->x / this->length(), this->y / this->length(), this->z / this->length()};
	}
	
	inline Vector3<T> operator+(const Vector3<T>& rhs) const
	{
		return {this->x + rhs.getX(), this->y + rhs.getY(), this->z + rhs.getZ()};
	}
	
	inline Vector3<T> operator-(const Vector3<T>& rhs) const
	{
		return {this->x - rhs.getX(), this->y - rhs.getY(), this->z - rhs.getZ()};
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
		return (this->x == rhs.getX()) && (this->y == rhs.getY()) && (this->z == rhs.getZ());
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
	
	inline Vector4POD toRaw() const
	{
		Vector4POD raw;
		raw.x = this->getX();
		raw.y = this->getY();
		raw.z = this->getZ();
		raw.w = this->getW();
		return raw;
	}
	
	inline const T& getW() const
	{
		return this->w;
	}
	
	inline T& getWR()
	{
		return this->w;
	}
	
	inline T length() const
	{
		return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z) + (this->w * this->w));
	}
	
	inline T dot(Vector4<T> rhs) const
	{
		return (this->x * rhs.getX()) + (this->y * rhs.getY()) + (this->z * rhs.getZ() + (this->w * rhs.getW()));
	}
	
	inline Vector4<T> normalised() const
	{
		return {this->x / this->length(), this->y / this->length(), this->z / this->length(), this->w / this->length()};
	}
	
	inline Vector4<T> operator+(const Vector4<T>& rhs) const
	{
		return {this->x + rhs.getX(), this->y + rhs.getY(), this->z + rhs.getZ(), this->w + rhs.getW()};
	}
	
	inline Vector4<T> operator-(const Vector4<T>& rhs) const
	{
		return {this->x - rhs.getX(), this->y - rhs.getY(), this->z - rhs.getZ(), this->w - rhs.getW()};
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
		return (this->x == rhs.getX()) && (this->y == rhs.getY()) && (this->z == rhs.getZ()) && (this->w == rhs.getW());
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