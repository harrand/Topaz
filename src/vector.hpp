#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <array>
#include <functional>
#include <cmath>

/**
*C-style POD struct for a Vector2F
*/
struct Vector2POD
{
	/// x-component
	float x;
	/// y-component
	float y;
};

/**
 * C-style POD struct for a Vector3F
 */
struct Vector3POD
{
	/// x-component
	float x;
	/// y-component
	float y;
	/// z-component
	float z;
};

/**
 * C-style POD struct for a Vector4F
 */
struct Vector4POD
{
	float x;
	float y;
	float z;
	float w;
};

/**
 * Vector to hold a quantity of some value. It's a glorified array.
 * @tparam N - Number of elements
 * @tparam T - Type of element
 */
template<unsigned int N, typename T>
class Vector
{
public:
	/**
	 * Construct a Vector directly from an array.
	 * @param data - The data to be copied into the vector.
	 */
	constexpr Vector(std::array<T, N> data);
	/**
	 * Return magnitude of the Vector.
	 * @param sqrt_function - The function to use to perform a square-root on the type T. If none is provided, the default std::sqrt will be used.
	 * @return - Magnitude of the Vector.
	 */
	T length(std::function<T(T)> sqrt_function = std::sqrt) const;

	std::array<T, N> data;
};

/**
 * Spatial 2-dimensional Vector.
 * Subclass of a partial specialisation of Vector.
 * @tparam T - Type of element
 */
template<typename T>
class Vector2 : protected Vector<2, T>
{
public:
	/**
	 * Construct a 2-dimensional Vector directly from arguments.
	 * @param x - Representation of the x-coordinate.
	 * @param y - Representation of the y-coordinate.
	 */
	constexpr Vector2<T>(T x = T(), T y = T());
	/**
	 * Construct a 2-dimensional vector from an existing array.
	 * @param data - The array to be copied.
	 */
	constexpr Vector2<T>(const std::array<T, 2>& data);
	Vector2<T>(const Vector2<T>& copy);
	Vector2<T>(Vector2<T>&& move);
	~Vector2<T>() = default;
	Vector2<T>& operator=(const Vector2<T>& rhs);

	/**
	 * Converts the Vector2 to a POD type (if convertible).
	 * This is useful because sizeof(Vector2<T>) != 2 * sizeof(T)
	 * @return - The POD type packed only with the data.
	 */
	Vector2POD to_raw() const;
	/**
	 * Find the magnitude of the 2-dimensional Vector.
	 * @return - Magnitude of the 2-dimensional Vector.
	 */
	T length() const;
	/**
	 * Perform a 2-dimensional dot-product.
	 * @param rhs - The other 2-dimensional Vector to compute the dot-product from.
	 * @return - Dot product of this and the parameter.
	 */
	T dot(const Vector2<T>& rhs) const;
	/**
	 * Divide all data members by the magnitude, and return the copy.
	 * @return - Normalised version of this 2-dimensional Vector.
	 */
	Vector2<T> normalised() const;
	/**
	 * Add a pair of 2-dimensional Vectors.
	 * @param rhs - The other 2-dimensional Vector to perform the addition with.
	 * @return - this + the parameter.
	 */
	Vector2<T> operator+(const Vector2<T>& rhs) const;
	/**
	 * Subtract a pair of 2-dimensional Vectors.
	 * @param rhs - The other 2-dimensional Vector to perform the subtraction with.
	 * @return - this - the parameter.
	 */
	Vector2<T> operator-(const Vector2<T>& rhs) const;
	/**
	 * Multiply this 2-dimensional Vector by a scalar of the same underlying type. Return a copy of the result.
	 * @param scalar - The scalar to multiply this Vector by.
	 * @return - this * the scalar.
	 */
	Vector2<T> operator*(const T& scalar) const;
	/**
	 * Divide this 2-dimensional Vector by a scalar of the same underlying type. Return a copy of the result.
	 * @param scalar - The scalar to divide this Vector by.
	 * @return - this ÷ the parameter.
	 */
	Vector2<T> operator/(const T& scalar) const;
	/**
	 * Assign this 2-dimensional Vector to the addition of this and another 2-dimensional Vector.
	 * @param rhs - The other 2-dimensional Vector to perform the addition with.
	 * @return - this, where 'this = this + the parameter'.
	 */
	Vector2<T>& operator+=(const Vector2<T>& rhs);
	/**
	 * Assign this 2-dimensional Vector to the subtraction of this and another 2-dimensional Vector.
	 * @param rhs - The other 2-dimensional Vector to perform the subtraction with.
	 * @return - this, where 'this = this - the parameter'.
	 */
	Vector2<T>& operator-=(const Vector2<T>& rhs);
	/**
	 * Assign this 2-dimensional Vector to the multiplication of this and a scalar with the same underlying type.
	 * @param rhs - The scalar to perform the multiplication with.
	 * @return - this, where 'this = this * the parameter'.
	 */
	Vector2<T>& operator*=(const T& scalar);
	/**
	 * Assign this 2-dimensional Vector to the division of this and a scalar with the same underlying type.
	 * @param rhs - The scalar to perform the division with.
	 * @return - this, where 'this = this ÷ the parameter'.
	 */
	Vector2<T>& operator/=(const T& scalar);
	/**
	 * Compare this to another 2-dimensional Vector.
	 * @param rhs - The other 2-dimensional Vector to compare this to.
	 * @return - True if all data members of this are lesser than the data members of the parameter.
	 */
	bool operator<(const Vector2<T>& rhs) const;
	/**
	 * Compare this to another 2-dimensional Vector.
	 * @param rhs - The other 2-dimensional Vector to compare this to.
	 * @return - True if all data members of this are greater than the data members of the parameter.
	 */
	bool operator>(const Vector2<T>& rhs) const;
	/**
	 * Compare this to another 2-dimensional Vector.
	 * @param rhs - The other 2-dimensional Vector to compare this to.
	 * @return - True if all data members of this are lesser than or equal to the data members of the parameter.
	 */
	bool operator<=(const Vector2<T>& rhs) const;
	/**
	 * Compare this to another 2-dimensional Vector.
	 * @param rhs - The other 2-dimensional Vector to compare this to.
	 * @return - True if all data members of this are greater than or equal to the data members of the parameter.
	 */
	bool operator>=(const Vector2<T>& rhs) const;
	/**
	 * Equate this with another 2-dimensional Vector.
	 * @param rhs - The other 2-dimensional Vector to compare this to.
	 * @return - True if all data members of this are equal to the data members of the parameter.
	 */
	bool operator==(const Vector2<T>& rhs) const;
	/**
	 * Swizzle operator xy.
	 * @return - The 2-dimensional Vector [x, y]
	 */
	Vector2<T> xy() const;
	/**
	 * Swizzle operator yx.
	 * @return - The 2-dimensional Vector [y, x]
	 */
	Vector2<T> yx() const;

	T& x;
	T& y;
};

/**
 * Spatial 3-dimensional Vector.
 * Subclass of a partial specialisation of Vector.
 * @tparam T - Type of element
 */
template<typename T>
class Vector3: protected Vector<3, T>
{
public:
	/**
	 * Construct a 3-dimensional Vector directly from arguments.
	 * @param x - Representation of the x-coordinate.
	 * @param y - Representation of the y-coordinate.
	 * @param z - Representation of the z-coordinate.
	 */
	constexpr Vector3<T>(T x = T(), T y = T(), T z = T());
	/**
	 * Construct a 3-dimensional Vector via concatenation of a 2-dimensional Vector and a scalar.
	 * @param xy - Beginning 2-dimensional Vector component.
	 * @param z - Ending scalar component.
	 */
	constexpr Vector3<T>(Vector2<T> xy, T z);
	/**
	 * Construct a 3-dimensional Vector via concatenation of a scalar and a 2-dimensional Vector.
	 * @param x - Beginning scalar component.
	 * @param yz - Ending 2-dimensional Vector component.
	 */
	constexpr Vector3<T>(T x, Vector2<T> yz);
	/**
	 * Construct a 3-dimensional vector from an existing array.
	 * @param data - The array to be copied.
	 */
	constexpr Vector3<T>(const std::array<T, 3>& data);
	Vector3<T>(const Vector3<T>& copy);
	Vector3<T>(Vector3<T>&& move);
	~Vector3<T>() = default;
	Vector3<T>& operator=(const Vector3<T>& rhs);

	/**
	 * Converts the Vector3 to a POD type (if convertible).
	 * This is useful because sizeof(Vector3<T>) != 3 * sizeof(T)
	 * @return - The POD type packed only with the data.
	 */
	Vector3POD to_raw() const;
	/**
	 * Find the magnitude of the 3-dimensional Vector.
	 * @return - Magnitude of the 3-dimensional Vector.
	 */
	T length() const;
	/**
	 * Perform a 3-dimensional dot-product.
	 * @param rhs - The other 3-dimensional Vector to compute the dot-product from.
	 * @return - Dot product of this and the parameter.
	 */
	T dot(const Vector3<T>& rhs) const;
	/**
	 * Perform a 3-dimensional cross-product.
	 * @param rhs - The other 3-dimensional Vector to compute the cross-product from.
	 * @return - Cross product of this and the parameter.
	 */
	Vector3<T> cross(const Vector3<T>& rhs) const;
	/**
	 * Divide all data members by the magnitude, and return the copy.
	 * @return - Normalised version of this 3-dimensional Vector.
	 */
	Vector3<T> normalised() const;
	/**
	 * Add a pair of 3-dimensional Vectors.
	 * @param rhs - The other 3-dimensional Vector to perform the addition with.
	 * @return - this + the parameter.
	 */
	Vector3<T> operator+(const Vector3<T>& rhs) const;
	/**
	 * Subtract a pair of 3-dimensional Vectors.
	 * @param rhs - The other 3-dimensional Vector to perform the subtraction with.
	 * @return - this - the parameter.
	 */
	Vector3<T> operator-(const Vector3<T>& rhs) const;
	/**
	 * Multiply this 3-dimensional Vector by a scalar of the same underlying type. Return a copy of the result.
	 * @param scalar - The scalar to multiply this Vector by.
	 * @return - this * the scalar.
	 */
	Vector3<T> operator*(const T& scalar) const;
	/**
	 * Divide this 3-dimensional Vector by a scalar of the same underlying type. Return a copy of the result.
	 * @param scalar - The scalar to divide this Vector by.
	 * @return - this ÷ the parameter.
	 */
	Vector3<T> operator/(const T& scalar) const;
	/**
	 * Assign this 3-dimensional Vector to the addition of this and another 3-dimensional Vector.
	 * @param rhs - The other 3-dimensional Vector to perform the addition with.
	 * @return - this, where 'this = this + the parameter'.
	 */
	Vector3<T>& operator+=(const Vector3<T>& rhs);
	/**
	 * Assign this 3-dimensional Vector to the subtraction of this and another 3-dimensional Vector.
	 * @param rhs - The other 3-dimensional Vector to perform the subtraction with.
	 * @return - this, where 'this = this - the parameter'.
	 */
	Vector3<T>& operator-=(const Vector3<T>& rhs);
	/**
	 * Assign this 3-dimensional Vector to the multiplication of this and a scalar with the same underlying type.
	 * @param rhs - The scalar to perform the multiplication with.
	 * @return - this, where 'this = this * the parameter'.
	 */
	Vector3<T>& operator*=(const T& scalar);
	/**
	 * Assign this 3-dimensional Vector to the division of this and a scalar with the same underlying type.
	 * @param rhs - The scalar to perform the division with.
	 * @return - this, where 'this = this ÷ the parameter'.
	 */
	Vector3<T>& operator/=(const T& scalar);
	/**
	 * Compare this to another 3-dimensional Vector.
	 * @param rhs - The other 3-dimensional Vector to compare this to.
	 * @return - True if all data members of this are lesser than the data members of the parameter.
	 */
	bool operator<(const Vector3<T>& rhs) const;
	/**
	 * Compare this to another 3-dimensional Vector.
	 * @param rhs - The other 3-dimensional Vector to compare this to.
	 * @return - True if all data members of this are greater than the data members of the parameter.
	 */
	bool operator>(const Vector3<T>& rhs) const;
	/**
	 * Compare this to another 3-dimensional Vector.
	 * @param rhs - The other 3-dimensional Vector to compare this to.
	 * @return - True if all data members of this are lesser than or equal to the data members of the parameter.
	 */
	bool operator<=(const Vector3<T>& rhs) const;
	/**
	 * Compare this to another 3-dimensional Vector.
	 * @param rhs - The other 3-dimensional Vector to compare this to.
	 * @return - True if all data members of this are greater than or equal to the data members of the parameter.
	 */
	bool operator>=(const Vector3<T>& rhs) const;
	/**
	 * Equate this with another 3-dimensional Vector.
	 * @param rhs - The other 3-dimensional Vector to compare this to.
	 * @return - True if all data members of this are equal to the data members of the parameter.
	 */
	bool operator==(const Vector3<T>& rhs) const;
	/**
	 * Swizzle operator xy.
	 * @return - The 2-dimensional Vector [x, y]
	 */
	Vector2<T> xy() const;
	/**
	 * Swizzle operator yx.
	 * @return - The 2-dimensional Vector [y, x]
	 */
	Vector2<T> yx() const;
	/**
	 * Swizzle operator xyz.
	 * @return - The 3-dimensional Vector {x, y, z}
	 */
	Vector3<T> xyz() const;
	/**
	 * Swizzle operator xzy.
	 * @return - The 3-dimensional Vector {x, z, y}
	 */
	Vector3<T> xzy() const;
	/**
	 * Swizzle operator yxz.
	 * @return - The 3-dimensional Vector {y, x, z}
	 */
	Vector3<T> yxz() const;
	/**
	 * Swizzle operator yzx.
	 * @return - The 3-dimensional Vector {y, z, x}
	 */
	Vector3<T> yzx() const;
	/**
	 * Swizzle operator zxy.
	 * @return - The 3-dimensional Vector {z, x, y}
	 */
	Vector3<T> zxy() const;
	/**
	 * Swizzle operator zyx.
	 * @return - The 3-dimensional Vector {z, y, x}
	 */
	Vector3<T> zyx() const;

	T& x;
	T& y;
	T& z;
};

/**
 * Spatial 4-dimensional Vector.
 * Subclass of a partial specialisation of Vector.
 * @tparam T - Type of element
 */
template<typename T>
class Vector4: protected Vector<4, T>
{
public:
	/**
	 * Construct a 4-dimensional Vector directly from arguments.
	 * @param x - Representation of the x-coordinate.
	 * @param y - Representation of the y-coordinate.
	 * @param z - Representation of the z-coordinate.
	 * @param w - Representation of the w-coordinate.
	 */
	constexpr Vector4<T>(T x = T(), T y = T(), T z = T(), T w = T());
	/**
	 * Construct a 4-dimensional Vector via concatenation of a 3-dimensional Vector and a scalar.
	 * @param xyz - Beginning 3-dimensional Vector component.
	 * @param w - Ending scalar component.
	 */
	constexpr Vector4<T>(Vector3<T> xyz, T w);
	/**
	 * Construct a 4-dimensional Vector via concatenation of a scalar and a 3-dimensional Vector.
	 * @param x - Beginning scalar component.
	 * @param yzw - Ending 3-dimensional Vector component.
	 */
	constexpr Vector4<T>(T x, Vector3<T> yzw);
	/**
	 * Construct a 4-dimensional Vector via concatenation of a pair of 2-dimensional Vectors.
	 * @param xy - Beginning 2-dimensional Vector component.
	 * @param zw - Ending 2-dimensional Vector component.
	 */
	constexpr Vector4<T>(Vector2<T> xy, Vector2<T> zw);
	/**
	 * Construct a 4-dimensional vector from an existing array.
	 * @param data - The array to be copied.
	 */
	constexpr Vector4<T>(const std::array<T, 4>& data);
	Vector4<T>(const Vector4<T>& copy);
	Vector4<T>(Vector4<T>&& move);
	~Vector4<T>() = default;
	Vector4<T>& operator=(const Vector4<T>& rhs);

	/**
	 * Converts the Vector4 to a POD type (if convertible).
	 * This is useful because sizeof(Vector4<T>) != 4 * sizeof(T)
	 * @return - The POD type packed only with the data.
	 */
	Vector4POD to_raw() const;
	/**
	 * Find the magnitude of the 4-dimensional Vector.
	 * @return - Magnitude of the 4-dimensional Vector.
	 */
	T length() const;
	/**
	 * Perform a 4-dimensional dot-product.
	 * @param rhs - The other 4-dimensional Vector to compute the dot-product from.
	 * @return - Dot product of this and the parameter.
	 */
	T dot(Vector4<T> rhs) const;
	/**
	 * Divide all data members by the magnitude, and return the copy.
	 * @return - Normalised version of this 4-dimensional Vector.
	 */
	Vector4<T> normalised() const;
	/**
	 * Add a pair of 4-dimensional Vectors.
	 * @param rhs - The other 4-dimensional Vector to perform the addition with.
	 * @return - this + the parameter.
	 */
	Vector4<T> operator+(const Vector4<T>& rhs) const;
	/**
	 * Subtract a pair of 4-dimensional Vectors.
	 * @param rhs - The other 4-dimensional Vector to perform the subtraction with.
	 * @return - this - the parameter.
	 */
	Vector4<T> operator-(const Vector4<T>& rhs) const;
	/**
	 * Assign this 4-dimensional Vector to the multiplication of this and a scalar with the same underlying type.
	 * @param rhs - The scalar to perform the multiplication with.
	 * @return - this, where 'this = this * the parameter'.
	 */
	Vector4<T> operator*(const T& scalar) const;
	/**
	 * Assign this 4-dimensional Vector to the division of this and a scalar with the same underlying type.
	 * @param rhs - The scalar to perform the division with.
	 * @return - this, where 'this = this ÷ the parameter'.
	 */
	Vector4<T> operator/(const T& scalar) const;
	/**
	 * Assign this 4-dimensional Vector to the addition of this and another 4-dimensional Vector.
	 * @param rhs - The other 4-dimensional Vector to perform the addition with.
	 * @return - this, where 'this = this + the parameter'.
	 */
	Vector4<T>& operator+=(const Vector4<T>& rhs);
	/**
	 * Assign this 4-dimensional Vector to the subtraction of this and another 4-dimensional Vector.
	 * @param rhs - The other 4-dimensional Vector to perform the subtraction with.
	 * @return - this, where 'this = this - the parameter'.
	 */
	Vector4<T>& operator-=(const Vector4<T>& rhs);
	/**
	 * Assign this 4-dimensional Vector to the multiplication of this and a scalar with the same underlying type.
	 * @param rhs - The scalar to perform the multiplication with.
	 * @return - this, where 'this = this * the parameter'.
	 */
	Vector4<T>& operator*=(const T& scalar);
	/**
	 * Assign this 4-dimensional Vector to the division of this and a scalar with the same underlying type.
	 * @param rhs - The scalar to perform the division with.
	 * @return - this, where 'this = this ÷ the parameter'.
	 */
	Vector4<T>& operator/=(const T& scalar);
	/**
	 * Compare this to another 4-dimensional Vector.
	 * @param rhs - The other 4-dimensional Vector to compare this to.
	 * @return - True if all data members of this are lesser than the data members of the parameter.
	 */
	bool operator<(const Vector4<T>& rhs) const;
	/**
	 * Compare this to another 4-dimensional Vector.
	 * @param rhs - The other 4-dimensional Vector to compare this to.
	 * @return - True if all data members of this are greater than the data members of the parameter.
	 */
	bool operator>(const Vector4<T>& rhs) const;
	/**
	 * Compare this to another 4-dimensional Vector.
	 * @param rhs - The other 4-dimensional Vector to compare this to.
	 * @return - True if all data members of this are lesser than or equal to the data members of the parameter.
	 */
	bool operator<=(const Vector4<T>& rhs) const;
	/**
	 * Compare this to another 4-dimensional Vector.
	 * @param rhs - The other 4-dimensional Vector to compare this to.
	 * @return - True if all data members of this are greater than or equal to the data members of the parameter.
	 */
	bool operator>=(const Vector4<T>& rhs) const;
	/**
	 * Equate this with another 4-dimensional Vector.
	 * @param rhs - The other 4-dimensional Vector to compare this to.
	 * @return - True if all data members of this are equal to the data members of the parameter.
	 */
	bool operator==(const Vector4<T>& rhs) const;
	/**
	 * Swizzle operator xy.
	 * @return - The 2-dimensional Vector [x, y]
	 */
	Vector2<T> xy() const;
	/**
	 * Swizzle operator yx.
	 * @return - The 2-dimensional Vector [y, x]
	 */
	Vector2<T> yx() const;
	/**
	 * Swizzle operator xyz.
	 * @return - The 3-dimensional Vector {x, y, z}
	 */
	Vector3<T> xyz() const;
	/**
	 * Swizzle operator xzy.
	 * @return - The 3-dimensional Vector {x, z, y}
	 */
	Vector3<T> xzy() const;
	/**
	 * Swizzle operator yxz.
	 * @return - The 3-dimensional Vector {y, x, z}
	 */
	Vector3<T> yxz() const;
	/**
	 * Swizzle operator yzx.
	 * @return - The 3-dimensional Vector {y, z, x}
	 */
	Vector3<T> yzx() const;
	/**
	 * Swizzle operator zxy.
	 * @return - The 3-dimensional Vector {z, x, y}
	 */
	Vector3<T> zxy() const;
	/**
	 * Swizzle operator zyx.
	 * @return - The 3-dimensional Vector {z, y, x}
	 */
	Vector3<T> zyx() const;
	/**
	 * Swizzle operator xyzw.
	 * @return - The 4-dimensional Vector {x, y, z, w}
	 */
	Vector4<T> xyzw() const;
	/**
	 * Swizzle operator xywz.
	 * @return - The 4-dimensional Vector {x, y, w, z}
	 */
	Vector4<T> xywz() const;
	/**
	 * Swizzle operator xzyw.
	 * @return - The 4-dimensional Vector {x, z, y, w}
	 */
	Vector4<T> xzyw() const;
	/**
	 * Swizzle operator xzwy.
	 * @return - The 4-dimensional Vector {x, z, w, y}
	 */
	Vector4<T> xzwy() const;
	/**
	 * Swizzle operator xwyz.
	 * @return - The 4-dimensional Vector {x, w, y, z}
	 */
	Vector4<T> xwyz() const;
	/**
	 * Swizzle operator xwzy.
	 * @return - The 4-dimensional Vector {x, w, z, y}
	 */
	Vector4<T> xwzy() const;
	/**
	 * Swizzle operator yxzw.
	 * @return - The 4-dimensional Vector {y, x, z, w}
	 */
	Vector4<T> yxzw() const;
	/**
	 * Swizzle operator yxwz.
	 * @return - The 4-dimensional Vector {y, x, w, z}
	 */
	Vector4<T> yxwz() const;
	/**
	 * Swizzle operator yzxw.
	 * @return - The 4-dimensional Vector {y, z, x, w}
	 */
	Vector4<T> yzxw() const;
	/**
	 * Swizzle operator yzwx.
	 * @return - The 4-dimensional Vector {y, z, w, x}
	 */
	Vector4<T> yzwx() const;
	/**
	 * Swizzle operator ywxz.
	 * @return - The 4-dimensional Vector {y, w, x, z}
	 */
	Vector4<T> ywxz() const;
	/**
	 * Swizzle operator ywzx.
	 * @return - The 4-dimensional Vector {y, w, z, x}
	 */
	Vector4<T> ywzx() const;
	/**
	 * Swizzle operator zxyw.
	 * @return - The 4-dimensional Vector {z, x, y, w}
	 */
	Vector4<T> zxyw() const;
	/**
	 * Swizzle operator zxwy.
	 * @return - The 4-dimensional Vector {zxwy}
	 */
	Vector4<T> zxwy() const;
	/**
	 * Swizzle operator zyxw.
	 * @return - The 4-dimensional Vector {z, y, x, w}
	 */
	Vector4<T> zyxw() const;
	/**
	 * Swizzle operator zywx.
	 * @return - The 4-dimensional Vector {z, y, w, x}
	 */
	Vector4<T> zywx() const;
	/**
	 * Swizzle operator zwxy.
	 * @return - The 4-dimensional Vector {z, w, x, y}
	 */
	Vector4<T> zwxy() const;
	/**
	 * Swizzle operator zwyx.
	 * @return - The 4-dimensional Vector {z, w, y, x}
	 */
	Vector4<T> zwyx() const;
	/**
	 * Swizzle operator wxyz.
	 * @return - The 4-dimensional Vector {w, x, y, z}
	 */
	Vector4<T> wxyz() const;
	/**
	 * Swizzle operator wxzy.
	 * @return - The 4-dimensional Vector {w, x, z, y}
	 */
	Vector4<T> wxzy() const;
	/**
	 * Swizzle operator wyxz.
	 * @return - The 4-dimensional Vector {w, y, x, z}
	 */
	Vector4<T> wyxz() const;
	/**
	 * Swizzle operator wyzx.
	 * @return - The 4-dimensional Vector {w, y, z, x}
	 */
	Vector4<T> wyzx() const;
	/**
	 * Swizzle operator wzxy.
	 * @return - The 4-dimensional Vector {w, z, x, y}
	 */
	Vector4<T> wzxy() const;
	/**
	 * Swizzle operator wzyx.
	 * @return - The 4-dimensional Vector {w, z, y, x}
	 */
	Vector4<T> wzyx() const;

	T& x;
	T& y;
	T& z;
	T& w;
};

#include "vector.inl"
/**
 * Represents a 2-dimensional Vector of floats.
 */
using Vector2F = Vector2<float>;
/**
 * Represents a 3-dimensional Vector of floats.
 */
using Vector3F = Vector3<float>;
/**
 * Represents a 4-dimensional Vector of floats.
 */
using Vector4F = Vector4<float>;
#endif