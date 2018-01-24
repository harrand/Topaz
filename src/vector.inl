#include <cmath>

template<typename T>
Vector2<T>::Vector2(T x, T y): x(x), y(y){}
template<typename T>
constexpr Vector2<T>::Vector2(const std::array<T, 2>& data): x(data[0]), y(data[1]){}

template<typename T>
Vector2POD Vector2<T>::to_raw() const
{
	Vector2POD raw;
	raw.x = this->x;
	raw.y = this->y;
	return raw;
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
	return {this->x + rhs.x, this->y + rhs.y};
}

template<typename T>
Vector2<T> Vector2<T>::operator-(const Vector2<T>& rhs) const
{
	return {this->x - rhs.x, this->y - rhs.y};
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
Vector3<T>::Vector3(T x, T y, T z): Vector2<T>(x, y), z(z){}
template<typename T>
Vector3<T>::Vector3(Vector2<T> xy, T z): Vector2<T>(xy), z(z){}
template<typename T>
Vector3<T>::Vector3(T x, Vector2<T> yz): Vector2<T>(x, yz.x), z(yz.y){}
template<typename T>
constexpr Vector3<T>::Vector3(const std::array<T, 3>& data): Vector2<T>(std::array<T, 2>({data[0], data[1]})), z(data[2]){}

template<typename T>
Vector3POD Vector3<T>::to_raw() const
{
	Vector3POD raw;
	raw.x = this->x;
	raw.y = this->y;
	raw.z = this->z;
	return raw;
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
Vector3<T> Vector3<T>::normalised() const
{
	if(this->length() == 0)
		return {};
	return {this->x / this->length(), this->y / this->length(), this->z / this->length()};
}

template<typename T>
Vector3<T> Vector3<T>::operator+(const Vector3<T>& rhs) const
{
	return {this->x + rhs.x, this->y + rhs.y, this->z + rhs.z};
}

template<typename T>
Vector3<T> Vector3<T>::operator-(const Vector3<T>& rhs) const
{
	return {this->x - rhs.x, this->y - rhs.y, this->z - rhs.z};
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
	return (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z);
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
Vector4<T>::Vector4(T x, Vector3<T> yzw): Vector3<T>(x, yzw.x, yzw.y), w(yzw.z){}
template<typename T>
Vector4<T>::Vector4(Vector2<T> xy, Vector2<T> zw): Vector3<T>(xy.x, xy.y, zw.x), w(zw.y){}
template<typename T>
constexpr Vector4<T>::Vector4(const std::array<T, 4>& data): Vector3<T>(std::array<T, 3>({data[0], data[1], data[2]})), w(data[3]){}

template<typename T>
Vector4POD Vector4<T>::to_raw() const
{
	Vector4POD raw;
	raw.x = this->x;
	raw.y = this->y;
	raw.z = this->z;
	raw.w = this->w;
	return raw;
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
	return (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z) && (this->w == rhs.w);
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