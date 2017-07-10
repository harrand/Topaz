#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <cmath>
#include <vector>

struct VectorS2F
{
	float x, y;
};

struct VectorS3F
{
	float x, y, z;
};

struct VectorS4F
{
	float x, y, z, w;
};

template<typename T>
class Vector2
{
public:
	inline Vector2<T>(T x = T(), T y = T()): x(x), y(y){}
	inline Vector2<T>(const Vector2<T>& copy) = default;
	inline Vector2<T>(Vector2<T>&& move) = default;
	inline Vector2<T>& operator=(const Vector2<T>& rhs) = default;
	
	inline VectorS2F toRaw() const
	{
		VectorS2F raw;
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
	inline virtual T length() const
	{
		return sqrt((this->x * this->x) + (this->y * this->y));
	}
	inline virtual T dot(Vector2<T> rhs) const
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
		return (this->x == rhs.getX()) && (this->y == rhs.getY());
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
	inline Vector3<T>(const Vector3<T>& copy) = default;
	inline Vector3<T>(Vector3<T>&& move) = default;
	inline Vector3<T>& operator=(const Vector3<T>& rhs) = default;
	
	inline VectorS3F toRaw() const
	{
		VectorS3F raw;
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
	inline virtual T length() const
	{
		return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
	}
	inline virtual T dot(Vector3<T> rhs) const
	{
		return (this->x * rhs.getX()) + (this->y * rhs.getY()) + (this->z * rhs.getZ());
	}
	inline virtual Vector3<T> cross(Vector3<T> rhs) const
	{
		return Vector3<T>((this->y * rhs.getZ()) - (this->z * rhs.getY()), (this->z * rhs.getX()) - (this->x * rhs.getZ()), (this->x * rhs.getY()) - (this->y * rhs.getX()));
	}
	inline Vector3<T> normalised() const
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
	inline Vector4<T>(const Vector4& copy) = default;
	inline Vector4<T>(Vector4&& move) = default;
	inline Vector4<T>& operator=(const Vector4<T>& rhs) = default;
	
	inline VectorS4F toRaw() const
	{
		VectorS4F raw;
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
	inline virtual T length() const
	{
		return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z) + (this->w * this->w));
	}
	inline virtual T dot(Vector4<T> rhs) const
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
protected:
	T w;
};


using Vector2F = Vector2<float>;
using Vector3F = Vector3<float>;
using Vector4F = Vector4<float>;
#endif