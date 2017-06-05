#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <cmath>
#include <vector>

template<typename T>
class Vector2
{
public:
	inline Vector2<T>(T x = 0.0f, T y = 0.0f): x(x), y(y){}
	inline Vector2<T>(const Vector2<T>& copy) = default;
	inline Vector2<T>(Vector2<T>&& move) = default;
	inline Vector2<T>& operator=(const Vector2<T>& rhs) = default;
	
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
	inline T dot(Vector2<T> rhs) const
	{
		return (this->x * rhs.getX()) + (this->y * rhs.getY());
	}
	inline Vector2<T> normalised() const
	{
		return Vector2<T>(this->x / this->length(), this->y / this->length());
	}
	
	inline Vector2<T> operator+(const Vector2<T>& rhs) const
	{
		return Vector2<T>(this->x + rhs.getX(), this->y + rhs.getY());
	}
	inline Vector2<T> operator-(const Vector2<T>& rhs) const
	{
		return Vector2<T>(this->x - rhs.getX(), this->y - rhs.getY());
	}
	inline Vector2<T> operator*(T scalar) const
	{
		return Vector2<T>(this->x * scalar, this->y * scalar);
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
		return this->length() == rhs.length();
	}
private:
	T x, y;
};

template<typename T>
class Vector3
{
public:
	inline Vector3<T>(T x = 0.0f, T y = 0.0f, T z = 0.0f): x(x), y(y), z(z){}
	inline Vector3<T>(const Vector3<T>& copy) = default;
	inline Vector3<T>(Vector3<T>&& move) = default;
	inline Vector3<T>& operator=(const Vector3<T>& rhs) = default;
	
	inline const T& getX() const
	{
		return this->x;
	}
	inline const T& getY() const
	{
		return this->y;
	}
	inline const T& getZ() const
	{
		return this->z;
	}
	inline T& getXR()
	{
		return this->x;
	}
	inline T& getYR()
	{
		return this->y;
	}
	inline T& getZR()
	{
		return this->z;
	}
	inline T length() const
	{
		return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
	}
	inline T dot(Vector3<T> rhs) const
	{
		return (this->x * rhs.getX()) + (this->y * rhs.getY()) + (this->z * rhs.getZ());
	}
	inline Vector3 cross(Vector3<T> rhs) const
	{
		return Vector3<T>((this->y * rhs.getZ()) - (this->z * rhs.getY()), (this->z * rhs.getX()) - (this->x * rhs.getZ()), (this->x * rhs.getY()) - (this->y * rhs.getX()));
	}
	inline Vector3 normalised() const
	{
		return Vector3<T>(this->x / this->length(), this->y / this->length(), this->z / this->length());
	}
	
	inline Vector3<T> operator+(const Vector3<T>& rhs) const
	{
		return Vector3<T>(this->x + rhs.getX(), this->y + rhs.getY(), this->z + rhs.getZ());
	}
	inline Vector3<T> operator-(const Vector3<T>& rhs) const
	{
		return Vector3<T>(this->x - rhs.getX(), this->y - rhs.getY(), this->z - rhs.getZ());
	}
	inline Vector3<T> operator*(T scalar) const
	{
		return Vector3<T>(this->x * scalar, this->y * scalar, this->z * scalar);
	}
	inline Vector3<T> operator/(T scalar) const
	{
		return Vector3<T>(this->x / scalar, this->y / scalar, this->z / scalar);
	}
	inline bool operator<(const Vector3<T>& rhs) const
	{
		return this->length() < rhs.length();
	}
	inline bool operator>(const Vector3<T>& rhs) const
	{
		return this->length() > rhs.length();
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
private:
	T x, y, z;
};

template<typename T>
class Vector4
{
public:
	inline Vector4<T>(T x = 0.0f, T y = 0.0f, T z = 0.0f, T w = 0.0f): x(x), y(y), z(z), w(w){}
	inline Vector4<T>(const Vector4& copy) = default;
	inline Vector4<T>(Vector4&& move) = default;
	inline Vector4<T>& operator=(const Vector4<T>& rhs) = default;
	
	inline const T& getX() const
	{
		return this->x;
	}
	inline const T& getY() const
	{
		return this->y;
	}
	inline const T& getZ() const
	{
		return this->z;
	}
	inline const T& getW() const
	{
		return this->w;
	}
	inline T& getXR()
	{
		return this->x;
	}
	inline T& getYR()
	{
		return this->y;
	}
	inline T& getZR()
	{
		return this->z;
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
		return Vector4<T>(this->x / this->length(), this->y / this->length(), this->z / this->length(), this->w / this->length());
	}
	
	inline Vector4<T> operator+(const Vector4<T>& rhs) const
	{
		return Vector4<T>(this->x + rhs.getX(), this->y + rhs.getY(), this->z + rhs.getZ(), this->w + rhs.getW());
	}
	inline Vector4<T> operator-(const Vector4<T>& rhs) const
	{
		return Vector4<T>(this->x - rhs.getX(), this->y - rhs.getY(), this->z - rhs.getZ(), this->w - rhs.getW());
	}
	inline Vector4<T> operator*(T scalar) const
	{
		return Vector4<T>(this->x * scalar, this->y * scalar, this->z * scalar, this->w * scalar);
	}
	inline Vector4<T> operator/(T scalar) const
	{
		return Vector4<T>(this->x / scalar, this->y / scalar, this->z / scalar, this->w / scalar);
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