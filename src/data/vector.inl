#include <cmath>
#include <iostream>
#include "utility/generic.hpp"

template<unsigned int N, typename T>
constexpr Vector<N, T>::Vector(std::array<T, N> underlying_data): underlying_data(underlying_data){}

template<unsigned int N, typename T>
T Vector<N, T>::length(std::function<T(T)> sqrt_function) const
{
	T squared_total = T();
	for(const auto& value : this->underlying_data)
		squared_total += value * value;
	return sqrt_function(squared_total);
}

template<unsigned int N, typename T>
const std::array<T, N>& Vector<N, T>::data() const
{
    return this->underlying_data;
}

template<unsigned int N, typename T>
Vector<N, T> Vector<N, T>::lerp(const Vector<N, T>& rhs, double proportion) const
{
	Vector<N, T> result{std::array<T, N>{T{}}};
	using namespace tz::utility;
	for(std::size_t i = 0; i < N; i++)
		result.underlying_data[i] = numeric::linear_interpolate(this->data()[i], rhs.data()[i], proportion);
	return result;
}

template<unsigned int N, typename T>
bool Vector<N, T>::operator==(const Vector<N, T>& rhs) const
{
	return this->underlying_data == rhs.underlying_data;
}

template<unsigned int N, typename T>
bool Vector<N, T>::operator!=(const Vector<N, T>& rhs) const
{
	return this->underlying_data != rhs.underlying_data;
}

template<unsigned int N, typename T>
Vector<N, T>::operator std::string() const
{
    std::string res = "[";
    for(unsigned int i = 0; i < N; i++)
    {
        res += tz::utility::generic::cast::to_string(this->underlying_data[i]);
        if(i != N - 1)
            res += ", ";
    }
    res += "]";
    return res;
}

template<unsigned int N, typename T>
std::ostream& operator<<(std::ostream& os, const Vector<N, T>& vector)
{
	os << static_cast<std::string>(vector);
	return os;
}

template<typename T>
constexpr Vector2<T>::Vector2(T x, T y): Vector<2, T>({x, y}), x(this->underlying_data[0]), y(this->underlying_data[1]){}
template<typename T>
constexpr Vector2<T>::Vector2(const std::array<T, 2>& data): Vector2<T>(data[0], data[1]){}

template<typename T>
Vector2<T>::Vector2(const Vector2<T>& copy): Vector2<T>(copy.x, copy.y){}

template<typename T>
Vector2<T>::Vector2(Vector2<T>&& move): Vector<2, T>(std::move(move.underlying_data)), x(this->underlying_data[0]), y(this->underlying_data[1]){}

template<typename T>
Vector2<T>& Vector2<T>::operator=(const Vector2<T>& rhs)
{
	/// this->x and this->y should still refer to this->data[0] and this->data[1] respectively.
	this->underlying_data = rhs.underlying_data;
	return *(this);
}

template<typename T>
Vector2<T>& Vector2<T>::operator=(const Vector<2, T>& rhs)
{
    this->underlying_data = rhs.underlying_data;
    return *(this);
}

template<typename T>
T Vector2<T>::length() const
{
	return sqrt((this->x * this->x) + (this->y * this->y));
}

template<typename T>
T Vector2<T>::dot(const Vector2<T>& rhs) const
{
	return (this->x * rhs.x) + (this->y * rhs.y);
}

template<typename T>
Vector2<T> Vector2<T>::normalised() const
{
	if(this->length() == 0)
		return {};
	return {this->x / this->length(), this->y / this->length()};
}

template<typename T>
Vector2<T> Vector2<T>::operator+(const Vector2<T>& rhs) const
{
    Vector2<T> copy = *this;
    return copy += rhs;
}

template<typename T>
Vector2<T> Vector2<T>::operator-(const Vector2<T>& rhs) const
{
	Vector2<T> copy = *this;
	return copy -= rhs;
}

template<typename T>
Vector2<T> Vector2<T>::operator*(const T& scalar) const
{
	Vector2<T> copy = *this;
	return copy *= scalar;
}

template<typename T>
Vector2<T> Vector2<T>::operator/(const T& scalar) const
{
    Vector2<T> copy = *this;
    return copy /= scalar;
}

template<typename T>
Vector2<T>& Vector2<T>::operator+=(const Vector2<T>& rhs)
{
    this->x += rhs.x;
    this->y += rhs.y;
    return *this;
}

template<typename T>
Vector2<T>& Vector2<T>::operator-=(const Vector2<T>& rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	return *this;
}

template<typename T>
Vector2<T>& Vector2<T>::operator*=(const T& scalar)
{
	this->x *= scalar;
	this->y *= scalar;
	return *this;
}

template<typename T>
Vector2<T>& Vector2<T>::operator/=(const T& scalar)
{
    this->x /= scalar;
    this->y /= scalar;
    return *this;
}

template<typename T>
bool Vector2<T>::operator<(const Vector2<T>& rhs) const
{
	return this->x < rhs.x && this->y < rhs.y;
}

template<typename T>
bool Vector2<T>::operator>(const Vector2<T>& rhs) const
{
	return this->x > rhs.x && this->y > rhs.y;
}

template<typename T>
bool Vector2<T>::operator<=(const Vector2<T>& rhs) const
{
	return *this < rhs || *this == rhs;
}

template<typename T>
bool Vector2<T>::operator>=(const Vector2<T>& rhs) const
{
	return *this > rhs || *this == rhs;
}

template<typename T>
bool Vector2<T>::operator==(const Vector2<T>& rhs) const
{
	return (this->x == rhs.x) && (this->y == rhs.y);
}

template<typename T>
Vector2<T> Vector2<T>::xy() const
{
	return {this->x, this->y};
}

template<typename T>
Vector2<T> Vector2<T>::yx() const
{
	return {this->y, this->x};
}

template<typename T>
constexpr Vector3<T>::Vector3(T x, T y, T z): Vector<3, T>({x, y, z}), x(this->underlying_data[0]), y(this->underlying_data[1]), z(this->underlying_data[2]){}
template<typename T>
constexpr Vector3<T>::Vector3(Vector2<T> xy, T z): Vector3<T>(xy.x, xy.y, z){}
template<typename T>
constexpr Vector3<T>::Vector3(T x, Vector2<T> yz): Vector3<T>(x, yz.x, yz.y){}
template<typename T>
constexpr Vector3<T>::Vector3(const std::array<T, 3>& data): Vector3<T>(std::array<T, 2>({data[0], data[1]}), data[2]){}

template<typename T>
Vector3<T>::Vector3(const Vector3<T>& copy): Vector3<T>(copy.x, copy.y, copy.z){}

template<typename T>
Vector3<T>::Vector3(Vector3<T>&& move): Vector<3, T>(std::move(move.underlying_data)), x(this->underlying_data[0]), y(this->underlying_data[1]), z(this->underlying_data[2]){}

template<typename T>
Vector3<T>& Vector3<T>::operator=(const Vector3<T>& rhs)
{
	/// this->x and this->y should still refer to this->data[0] and this->data[1] respectively.
	this->underlying_data = rhs.underlying_data;
	return *(this);
}

template<typename T>
Vector3<T>& Vector3<T>::operator=(const Vector<3, T>& rhs)
{
    this->underlying_data = rhs.underlying_data;
    return *(this);
}

template<typename T>
T Vector3<T>::length() const 
{
	return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
}

template<typename T>
T Vector3<T>::dot(const Vector3<T>& rhs) const
{
	return (this->x * rhs.x) + (this->y * rhs.y) + (this->z * rhs.z);
}

template<typename T>
Vector3<T> Vector3<T>::cross(const Vector3<T>& rhs) const
{
	return {(this->y * rhs.z) - (this->z * rhs.y), (this->z * rhs.x) - (this->x * rhs.z), (this->x * rhs.y) - (this->y * rhs.x)};
}

template<typename T>
Vector3<T> Vector3<T>::reflect(const Vector3<T>& rhs) const
{
	return (*this) - (rhs * 2.0f * this->dot(rhs));
}

/*
genType glm::core::function::geometric::reflect	(	genType const & 	I,
														genType const & 	N
)
For the incident vector I and surface orientation N, returns the reflection direction : result = I - 2.0 * dot(N, I) * N.
*/

template<typename T>
Vector3<T> Vector3<T>::normalised() const
{
	if(this->length() == 0)
		return {};
	return {this->x / this->length(), this->y / this->length(), this->z / this->length()};
}

template<typename T>
Vector3<T> Vector3<T>::operator+(const Vector3<T>& rhs) const
{
    Vector3<T> copy = *this;
    return copy += rhs;
}

template<typename T>
Vector3<T> Vector3<T>::operator-(const Vector3<T>& rhs) const
{
    Vector3<T> copy = *this;
    return copy -= rhs;}

template<typename T>
Vector3<T> Vector3<T>::operator*(const T& scalar) const
{
    Vector3<T> copy = *this;
    return copy *= scalar;
}

template<typename T>
Vector3<T> Vector3<T>::operator/(const T& scalar) const
{
    Vector3<T> copy = *this;
    return copy /= scalar;
}

template<typename T>
Vector3<T>& Vector3<T>::operator+=(const Vector3<T>& rhs)
{
	this->x += rhs.x;
    this->y += rhs.y;
    this->z += rhs.z;
    return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator-=(const Vector3<T>& rhs)
{
    this->x -= rhs.x;
    this->y -= rhs.y;
    this->z -= rhs.z;
    return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator*=(const T& scalar)
{
    this->x *= scalar;
    this->y *= scalar;
    this->z *= scalar;
    return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator/=(const T& scalar)
{
    this->x /= scalar;
    this->y /= scalar;
    this->z /= scalar;
    return *this;
}

template<typename T>
bool Vector3<T>::operator<(const Vector3<T>& rhs) const
{
	return this->x < rhs.x && this->y < rhs.y && this->z < rhs.z;
}

template<typename T>
bool Vector3<T>::operator>(const Vector3<T>& rhs) const
{
	return this->x > rhs.x && this->y > rhs.y && this->z > rhs.z;
}

template<typename T>
bool Vector3<T>::operator<=(const Vector3<T>& rhs) const
{
	return *this < rhs || *this == rhs;
}

template<typename T>
bool Vector3<T>::operator>=(const Vector3<T>& rhs) const
{
	return *this > rhs || *this == rhs;
}

template<typename T>
bool Vector3<T>::operator==(const Vector3<T>& rhs) const
{
	return (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z);
}

template<typename T>
Vector2<T> Vector3<T>::xy() const
{
	return {this->x, this->y};
}

template<typename T>
Vector2<T> Vector3<T>::yx() const
{
	return {this->y, this->x};
}

template<typename T>
Vector3<T> Vector3<T>::xyz() const
{
	return {this->x, this->y, this->z};
}

template<typename T>
Vector3<T> Vector3<T>::xzy() const
{
	return {this->x, this->z, this->y};
}

template<typename T>
Vector3<T> Vector3<T>::yxz() const
{
	return {this->y, this->x, this->z};
}

template<typename T>
Vector3<T> Vector3<T>::yzx() const
{
	return {this->y, this->z, this->x};
}

template<typename T>
Vector3<T> Vector3<T>::zxy() const
{
	return {this->z, this->x, this->y};
}

template<typename T>
Vector3<T> Vector3<T>::zyx() const
{
	return {this->z, this->y, this->x};
}

template<typename T>
constexpr Vector4<T>::Vector4(T x, T y, T z, T w): Vector<4, T>({x, y, z, w}), x(this->underlying_data[0]), y(this->underlying_data[1]), z(this->underlying_data[2]), w(this->underlying_data[3]){}
template<typename T>
constexpr Vector4<T>::Vector4(Vector3<T> xyz, T w): Vector4<T>(xyz.x, xyz.y, xyz.z, w){}
template<typename T>
constexpr Vector4<T>::Vector4(T x, Vector3<T> yzw): Vector4<T>(x, yzw.x, yzw.y, yzw.z){}
template<typename T>
constexpr Vector4<T>::Vector4(Vector2<T> xy, Vector2<T> zw): Vector4<T>(xy.x, xy.y, zw.x, zw.y){}
template<typename T>
constexpr Vector4<T>::Vector4(const std::array<T, 4>& data): Vector4<T>(std::array<T, 3>({data[0], data[1], data[2]}), data[3]){}

template<typename T>
Vector4<T>::Vector4(const Vector4<T>& copy): Vector4<T>(copy.x, copy.y, copy.z, copy.w){}

template<typename T>
Vector4<T>::Vector4(Vector4<T>&& move): Vector<4, T>(std::move(move.underlying_data)), x(this->underlying_data[0]), y(this->underlying_data[1]), z(this->underlying_data[2]), w(this->underlying_data[3]){}

template<typename T>
Vector4<T>& Vector4<T>::operator=(const Vector4<T>& rhs)
{
	this->underlying_data = rhs.underlying_data;
	return *(this);
}

template<typename T>
Vector4<T>& Vector4<T>::operator=(const Vector<4, T>& rhs)
{
    this->underlying_data = rhs.underlying_data;
    return *(this);
}

template<typename T>
T Vector4<T>::length() const
{
	return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z) + (this->w * this->w));
}

template<typename T>
T Vector4<T>::dot(Vector4<T> rhs) const
{
	return (this->x * rhs.x) + (this->y * rhs.y) + (this->z * rhs.z + (this->w * rhs.w));
}

template<typename T>
Vector4<T> Vector4<T>::normalised() const
{
	if(this->length() == 0)
		return {};
	return {this->x / this->length(), this->y / this->length(), this->z / this->length(), this->w / this->length()};
}

template<typename T>
Vector4<T> Vector4<T>::operator+(const Vector4<T>& rhs) const
{
	return {this->x + rhs.x, this->y + rhs.y, this->z + rhs.z, this->w + rhs.w};
}

template<typename T>
Vector4<T> Vector4<T>::operator-(const Vector4<T>& rhs) const
{
	return {this->x - rhs.x, this->y - rhs.y, this->z - rhs.z, this->w - rhs.w};
}

template<typename T>
Vector4<T> Vector4<T>::operator*(const T& scalar) const
{
	return {this->x * scalar, this->y * scalar, this->z * scalar, this->w * scalar};
}

template<typename T>
Vector4<T> Vector4<T>::operator/(const T& scalar) const
{
	return {this->x / scalar, this->y / scalar, this->z / scalar, this->w / scalar};
}

template<typename T>
Vector4<T>& Vector4<T>::operator+=(const Vector4<T>& rhs)
{
	(*this) = (*this) + rhs;
	return (*this);
}

template<typename T>
Vector4<T>& Vector4<T>::operator-=(const Vector4<T>& rhs)
{
	(*this) = (*this) - rhs;
	return (*this);
}

template<typename T>
Vector4<T>& Vector4<T>::operator*=(const T& scalar)
{
	(*this) = (*this) * scalar;
	return (*this);
}

template<typename T>
Vector4<T>& Vector4<T>::operator/=(const T& scalar)
{
	(*this) = (*this) / scalar;
	return (*this);
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
bool Vector4<T>::operator<=(const Vector4<T>& rhs) const
{
	return *this < rhs || *this == rhs;
}

template<typename T>
bool Vector4<T>::operator>=(const Vector4<T>& rhs) const
{
	return *this > rhs || *this == rhs;
}

template<typename T>
bool Vector4<T>::operator==(const Vector4<T>& rhs) const
{
	return (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z) && (this->w == rhs.w);
}

template<typename T>
Vector2<T> Vector4<T>::xy() const
{
	return {this->x, this->y};
}

template<typename T>
Vector2<T> Vector4<T>::yx() const
{
	return {this->y, this->x};
}

template<typename T>
Vector3<T> Vector4<T>::xyz() const
{
	return {this->x, this->y, this->z};
}

template<typename T>
Vector3<T> Vector4<T>::xzy() const
{
	return {this->x, this->z, this->y};
}

template<typename T>
Vector3<T> Vector4<T>::yxz() const
{
	return {this->y, this->x, this->z};
}

template<typename T>
Vector3<T> Vector4<T>::yzx() const
{
	return {this->y, this->z, this->x};
}

template<typename T>
Vector3<T> Vector4<T>::zxy() const
{
	return {this->z, this->x, this->y};
}

template<typename T>
Vector3<T> Vector4<T>::zyx() const
{
	return {this->z, this->y, this->x};
}

template<typename T>
Vector4<T> Vector4<T>::xyzw() const
{
	return {this->x, this->y, this->z, this->w};
}

template<typename T>
Vector4<T> Vector4<T>::xywz() const
{
	return {this->x, this->y, this->w, this->z};
}

template<typename T>
Vector4<T> Vector4<T>::xzyw() const
{
	return {this->x, this->z, this->y, this->w};
}

template<typename T>
Vector4<T> Vector4<T>::xzwy() const
{
	return {this->x, this->z, this->w, this->y};
}

template<typename T>
Vector4<T> Vector4<T>::xwyz() const
{
	return {this->x, this->w, this->y, this->z};
}

template<typename T>
Vector4<T> Vector4<T>::xwzy() const
{
	return {this->x, this->w, this->z, this->y};
}

template<typename T>
Vector4<T> Vector4<T>::yxzw() const
{
	return {this->y, this->x, this->z, this->w};
}

template<typename T>
Vector4<T> Vector4<T>::yxwz() const
{
	return {this->y, this->x, this->w, this->z};
}

template<typename T>
Vector4<T> Vector4<T>::yzxw() const
{
	return {this->y, this->z, this->x, this->w};
}

template<typename T>
Vector4<T> Vector4<T>::yzwx() const
{
	return {this->y, this->z, this->w, this->x};
}

template<typename T>
Vector4<T> Vector4<T>::ywxz() const
{
	return {this->y, this->w, this->x, this->z};
}

template<typename T>
Vector4<T> Vector4<T>::ywzx() const
{
	return {this->y, this->w, this->z, this->x};
}

template<typename T>
Vector4<T> Vector4<T>::zxyw() const
{
	return {this->z, this->x, this->y, this->w};
}

template<typename T>
Vector4<T> Vector4<T>::zxwy() const
{
	return {this->z, this->x, this->w, this->y};
}

template<typename T>
Vector4<T> Vector4<T>::zyxw() const
{
	return {this->z, this->y, this->x, this->w};
}

template<typename T>
Vector4<T> Vector4<T>::zywx() const
{
	return {this->z, this->y, this->w, this->x};
}

template<typename T>
Vector4<T> Vector4<T>::zwxy() const
{
	return {this->z, this->w, this->x, this->y};
}

template<typename T>
Vector4<T> Vector4<T>::zwyx() const
{
	return {this->z, this->w, this->y, this->x};
}

template<typename T>
Vector4<T> Vector4<T>::wxyz() const
{
	return {this->w, this->x, this->y, this->z};
}

template<typename T>
Vector4<T> Vector4<T>::wxzy() const
{
	return {this->w, this->x, this->z, this->y};
}

template<typename T>
Vector4<T> Vector4<T>::wyxz() const
{
	return {this->w, this->y, this->x, this->z};
}

template<typename T>
Vector4<T> Vector4<T>::wyzx() const
{
	return {this->w, this->y, this->z, this->x};
}

template<typename T>
Vector4<T> Vector4<T>::wzxy() const
{
	return {this->w, this->z, this->x, this->y};
}

template<typename T>
Vector4<T> Vector4<T>::wzyx() const
{
	return {this->w, this->z, this->y, this->x};
}