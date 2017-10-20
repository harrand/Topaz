#include <cmath>

template<typename T>
Vector2<T>::Vector2(T x, T y): x(x), y(y){}
template<typename T>
constexpr Vector2<T>::Vector2(const std::array<T, 2>& data): x(data[0]), y(data[1]){}

template<typename T>
Vector2POD Vector2<T>::to_raw() const
{
	Vector2POD raw;
	raw.x = this->get_x();
	raw.y = this->get_y();
	return raw;
}

template<typename T>
const T& Vector2<T>::get_x() const
{
	return this->x;
}

template<typename T>
const T& Vector2<T>::get_y() const
{
	return this->y;
}

template<typename T>
void Vector2<T>::set_x(T x)
{
	this->x = x;
}

template<typename T>
void Vector2<T>::set_y(T y)
{
	this->y = y;
}

template<typename T>
T Vector2<T>::length() const
{
	return sqrt((this->x * this->x) + (this->y * this->y));
}

template<typename T>
T Vector2<T>::dot(const Vector2<T>& rhs) const
{
	return (this->x * rhs.get_x()) + (this->y * rhs.get_y());
}

template<typename T>
Vector2<T> Vector2<T>::normalised() const
{
	return {this->x / this->length(), this->y / this->length()};
}

template<typename T>
Vector2<T> Vector2<T>::operator+(const Vector2<T>& rhs) const
{
	return {this->x + rhs.get_x(), this->y + rhs.get_y()};
}

template<typename T>
Vector2<T> Vector2<T>::operator-(const Vector2<T>& rhs) const
{
	return {this->x - rhs.get_x(), this->y - rhs.get_y()};
}

template<typename T>
Vector2<T> Vector2<T>::operator*(T scalar) const
{
	return {this->x * scalar, this->y * scalar};
}

template<typename T>
Vector2<T> Vector2<T>::operator/(T scalar) const
{
	return {this->x / scalar, this->y / scalar};
}

template<typename T>
Vector2<T>& Vector2<T>::operator+=(const Vector2<T>& rhs)
{
	(*this) = (*this) + rhs;
	return (*this);
}

template<typename T>
Vector2<T>& Vector2<T>::operator-=(const Vector2<T>& rhs)
{
	(*this) = (*this) - rhs;
	return (*this);
}

template<typename T>
Vector2<T>& Vector2<T>::operator*=(T scalar)
{
	(*this) = (*this) * scalar;
	return (*this);
}

template<typename T>
Vector2<T>& Vector2<T>::operator/=(T scalar)
{
	(*this) = (*this) / scalar;
	return (*this);
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
	return (this->x == rhs.get_x()) && (this->y == rhs.get_y());
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
Vector3<T>::Vector3(T x, T y, T z): Vector2<T>(x, y), z(z){}
template<typename T>
Vector3<T>::Vector3(Vector2<T> xy, T z): Vector2<T>(xy), z(z){}
template<typename T>
Vector3<T>::Vector3(T x, Vector2<T> yz): Vector2<T>(x, yz.get_x()), z(yz.get_y()){}
template<typename T>
constexpr Vector3<T>::Vector3(const std::array<T, 3>& data): Vector2<T>(std::array<T, 2>({data[0], data[1]})), z(data[2]){}

template<typename T>
Vector3POD Vector3<T>::to_raw() const
{
	Vector3POD raw;
	raw.x = this->get_x();
	raw.y = this->get_y();
	raw.z = this->get_z();
	return raw;
}

template<typename T>
const T& Vector3<T>::get_z() const
{
	return this->z;
}

template<typename T>
void Vector3<T>::set_z(T z)
{
	this->z = z;
}

template<typename T>
T Vector3<T>::length() const 
{
	return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
}

template<typename T>
T Vector3<T>::dot(const Vector3<T>& rhs) const
{
	return (this->x * rhs.get_x()) + (this->y * rhs.get_y()) + (this->z * rhs.get_z());
}

template<typename T>
Vector3<T> Vector3<T>::cross(const Vector3<T>& rhs) const
{
	return {(this->y * rhs.get_z()) - (this->z * rhs.get_y()), (this->z * rhs.get_x()) - (this->x * rhs.get_z()), (this->x * rhs.get_y()) - (this->y * rhs.get_x())};
}

template<typename T>
Vector3<T> Vector3<T>::normalised() const
{
	return {this->x / this->length(), this->y / this->length(), this->z / this->length()};
}

template<typename T>
Vector3<T> Vector3<T>::operator+(const Vector3<T>& rhs) const
{
	return {this->x + rhs.get_x(), this->y + rhs.get_y(), this->z + rhs.get_z()};
}

template<typename T>
Vector3<T> Vector3<T>::operator-(const Vector3<T>& rhs) const
{
	return {this->x - rhs.get_x(), this->y - rhs.get_y(), this->z - rhs.get_z()};
}

template<typename T>
Vector3<T> Vector3<T>::operator*(T scalar) const
{
	return {this->x * scalar, this->y * scalar, this->z * scalar};
}

template<typename T>
Vector3<T> Vector3<T>::operator/(T scalar) const
{
	return {this->x / scalar, this->y / scalar, this->z / scalar};
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

template<typename T>
Vector3<T>& Vector3<T>::operator*=(T scalar)
{
	(*this) = ((*this) * scalar);
	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator/=(T scalar)
{
	(*this) = ((*this) / scalar);
	return *this;
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
	return (this->x == rhs.get_x()) && (this->y == rhs.get_y()) && (this->z == rhs.get_z());
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
Vector4<T>::Vector4(T x, T y, T z, T w): Vector3<T>(x, y, z), w(w){}
template<typename T>
Vector4<T>::Vector4(Vector3<T> xyz, T w): Vector3<T>(xyz), w(w){}
template<typename T>
Vector4<T>::Vector4(T x, Vector3<T> yzw): Vector3<T>(x, yzw.get_x(), yzw.get_y()), w(yzw.get_z()){}
template<typename T>
Vector4<T>::Vector4(Vector2<T> xy, Vector2<T> zw): Vector3<T>(xy.get_x(), xy.get_y(), zw.get_x()), w(zw.get_y()){}
template<typename T>
constexpr Vector4<T>::Vector4(const std::array<T, 4>& data): Vector3<T>(std::array<T, 3>({data[0], data[1], data[2]})), w(data[3]){}

template<typename T>
Vector4POD Vector4<T>::to_raw() const
{
	Vector4POD raw;
	raw.x = this->get_x();
	raw.y = this->get_y();
	raw.z = this->get_z();
	raw.w = this->get_w();
	return raw;
}

template<typename T>
const T& Vector4<T>::get_w() const
{
	return this->w;
}

template<typename T>
void Vector4<T>::set_w(T w)
{
	this->w = w;
}

template<typename T>
T Vector4<T>::length() const
{
	return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z) + (this->w * this->w));
}

template<typename T>
T Vector4<T>::dot(Vector4<T> rhs) const
{
	return (this->x * rhs.get_x()) + (this->y * rhs.get_y()) + (this->z * rhs.get_z() + (this->w * rhs.get_w()));
}

template<typename T>
Vector4<T> Vector4<T>::normalised() const
{
	return {this->x / this->length(), this->y / this->length(), this->z / this->length(), this->w / this->length()};
}

template<typename T>
Vector4<T> Vector4<T>::operator+(const Vector4<T>& rhs) const
{
	return {this->x + rhs.get_x(), this->y + rhs.get_y(), this->z + rhs.get_z(), this->w + rhs.get_w()};
}

template<typename T>
Vector4<T> Vector4<T>::operator-(const Vector4<T>& rhs) const
{
	return {this->x - rhs.get_x(), this->y - rhs.get_y(), this->z - rhs.get_z(), this->w - rhs.get_w()};
}

template<typename T>
Vector4<T> Vector4<T>::operator*(T scalar) const
{
	return {this->x * scalar, this->y * scalar, this->z * scalar, this->w * scalar};
}

template<typename T>
Vector4<T> Vector4<T>::operator/(T scalar) const
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
Vector4<T>& Vector4<T>::operator*=(T scalar)
{
	(*this) = (*this) * scalar;
	return (*this);
}

template<typename T>
Vector4<T>& Vector4<T>::operator/=(T scalar)
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
	return (this->x == rhs.get_x()) && (this->y == rhs.get_y()) && (this->z == rhs.get_z()) && (this->w == rhs.get_w());
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