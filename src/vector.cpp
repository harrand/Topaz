#include "vector.hpp"

// Vector2

template<typename T>
Vector2<T>::Vector2(T x, T y): x(x), y(y){}

template<typename T>
const T& Vector2<T>::getX() const
{
	return this->x;
}

template<typename T>
const T& Vector2<T>::getY() const
{
	return this->y;
}

template<typename T>
T& Vector2<T>::getXR()
{
	return this->x;
}

template<typename T>
T& Vector2<T>::getYR()
{
	return this->y;
}

template<typename T>
T Vector2<T>::length() const
{
	return sqrt((this->x * this->x) + (this->y * this->y));
}

template<typename T>
T Vector2<T>::dot(Vector2<T> rhs) const
{
	return (this->x * rhs.getX()) + (this->y * rhs.getY());
}

template<typename T>
Vector2<T> Vector2<T>::normalised() const
{
	return Vector2<T>(this->x / this->length(), this->y / this->length());
}

template<typename T>
Vector2<T> Vector2<T>::operator+(const Vector2<T>& rhs) const
{
	return Vector2<T>(this->x + rhs.getX(), this->y + rhs.getY());
}

template<typename T>
Vector2<T> Vector2<T>::operator-(const Vector2<T>& rhs) const
{
	return Vector2<T>(this->x - rhs.getX(), this->y - rhs.getY());
}

template<typename T>
Vector2<T> Vector2<T>::operator*(T scalar) const
{
	return Vector2<T>(this->x * scalar, this->y * scalar);
}

template<typename T>
bool Vector2<T>::operator<(const Vector2<T>& rhs) const
{
	return this->length() < rhs.length();
}

template<typename T>
bool Vector2<T>::operator>(const Vector2<T>& rhs) const
{
	return this->length() > rhs.length();
}

template<typename T>
bool Vector2<T>::operator==(const Vector2<T>& rhs) const
{
	return (this->x == rhs.getX()) && (this->y == rhs.getY());
}

// Vector3

template<typename T>
Vector3<T>::Vector3(T x, T y, T z): x(x), y(y), z(z){}

template<typename T>
const T& Vector3<T>::getX() const
{
	return this->x;
}

template<typename T>
const T& Vector3<T>::getY() const
{
	return this->y;
}

template<typename T>
const T& Vector3<T>::getZ() const
{
	return this->z;
}

template<typename T>
T& Vector3<T>::getXR()
{
	return this->x;
}

template<typename T>
T& Vector3<T>::getYR()
{
	return this->y;
}

template<typename T>
T& Vector3<T>::getZR()
{
	return this->z;
}

template<typename T>
T Vector3<T>::length() const
{
	return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
}

template<typename T>
T Vector3<T>::dot(Vector3<T> rhs) const
{
	return (this->x * rhs.getX()) + (this->y * rhs.getY()) + (this->z * rhs.getZ());
}

template<typename T>
Vector3<T> Vector3<T>::cross(Vector3<T> rhs) const
{
	return Vector3<T>((this->y * rhs.getZ()) - (this->z * rhs.getY()), (this->z * rhs.getX()) - (this->x * rhs.getZ()), (this->x * rhs.getY()) - (this->y * rhs.getX()));
}

template<typename T>
Vector3<T> Vector3<T>::normalised() const
{
	return Vector3<T>(this->x / this->length(), this->y / this->length(), this->z / this->length());
}

template<typename T>
Vector3<T> Vector3<T>::operator+(const Vector3<T>& rhs) const
{
	return Vector3<T>(this->x + rhs.getX(), this->y + rhs.getY(), this->z + rhs.getZ());
}

template<typename T>
Vector3<T> Vector3<T>::operator-(const Vector3<T>& rhs) const
{
	return Vector3<T>(this->x - rhs.getX(), this->y - rhs.getY(), this->z - rhs.getZ());
}

template<typename T>
Vector3<T> Vector3<T>::operator*(T scalar) const
{
	return Vector3<T>(this->x * scalar, this->y * scalar, this->z * scalar);
}

template<typename T>
Vector3<T> Vector3<T>::operator/(T scalar) const
{
	return Vector3<T>(this->x / scalar, this->y / scalar, this->z / scalar);
}

template<typename T>
bool Vector3<T>::operator<(const Vector3<T>& rhs) const
{
	return this->length() < rhs.length();
}

template<typename T>
bool Vector3<T>::operator>(const Vector3<T>& rhs) const
{
	return this->length() > rhs.length();
}

template<typename T>
bool Vector3<T>::operator==(const Vector3<T>& rhs) const
{
	return (this->x == rhs.getX()) && (this->y == rhs.getY()) && (this->z == rhs.getZ());
}

template<typename T>
Vector3<T>& Vector3<T>::operator+=(const Vector3<T>& rhs)
{
	(*this) = ((*this) + rhs);
	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator-=(const Vector3<T>& rhs)
{
	(*this) = ((*this) - rhs);
	return *this;
}

// Vector4
template<typename T>
Vector4<T>::Vector4(T x, T y, T z, T w): x(x), y(y), z(z), w(w){}

template<typename T>
const T& Vector4<T>::getX() const
{
	return this->x;
}

template<typename T>
const T& Vector4<T>::getY() const
{
	return this->y;
}

template<typename T>
const T& Vector4<T>::getZ() const
{
	return this->z;
}

template<typename T>
const T& Vector4<T>::getW() const
{
	return this->w;
}

template<typename T>
T& Vector4<T>::getXR()
{
	return this->x;
}

template<typename T>
T& Vector4<T>::getYR()
{
	return this->y;
}

template<typename T>
T& Vector4<T>::getZR()
{
	return this->z;
}

template<typename T>
T& Vector4<T>::getWR()
{
	return this->w;
}

template<typename T>
T Vector4<T>::length() const
{
	return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z) + (this->w * this->w));
}

template<typename T>
T Vector4<T>::dot(Vector4<T> rhs) const
{
	return (this->x * rhs.getX()) + (this->y * rhs.getY()) + (this->z * rhs.getZ() + (this->w * rhs.getW()));
}

template<typename T>
Vector4<T> Vector4<T>::normalised() const
{
	return Vector4<T>(this->x / this->length(), this->y / this->length(), this->z / this->length(), this->w / this->length());
}

template<typename T>
Vector4<T> Vector4<T>::operator+(const Vector4<T>& rhs) const
{
	return Vector4<T>(this->x + rhs.getX(), this->y + rhs.getY(), this->z + rhs.getZ(), this->w + rhs.getW());
}

template<typename T>
Vector4<T> Vector4<T>::operator-(const Vector4<T>& rhs) const
{
	return Vector4<T>(this->x - rhs.getX(), this->y - rhs.getY(), this->z - rhs.getZ(), this->w - rhs.getW());
}

template<typename T>
Vector4<T> Vector4<T>::operator*(T scalar) const
{
	return Vector4<T>(this->x * scalar, this->y * scalar, this->z * scalar, this->w * scalar);
}

template<typename T>
bool Vector4<T>::operator<(const Vector4<T>& rhs) const
{
	return this->length() < rhs.length();
}

template<typename T>
bool Vector4<T>::operator>(const Vector4<T>& rhs) const
{
	return this->length() > rhs.length();
}

template<typename T>
bool Vector4<T>::operator==(const Vector4<T>& rhs) const
{
	return (this->x == rhs.getX()) && (this->y == rhs.getY()) && (this->z == rhs.getZ()) && (this->w == rhs.getW());
}

template class Vector2<float>;
template class Vector2<unsigned int>;
template class Vector3<float>;
template class Vector3<unsigned int>;
template class Vector4<float>;
template class Vector4<unsigned int>;