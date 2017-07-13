#ifndef QUATERNION_HPP
#define QUATERNION_HPP
#include "matrix.hpp"

class Quaternion: private Vector4<float>
{
public:
	Quaternion(float x, float y, float z, float w);
	Quaternion(const Vector3F& euler_rotation);
	Quaternion(const Vector3F& rotation_axis, float angle);
	Quaternion(const Matrix4x4& rotation);
	
	Quaternion(const Quaternion& copy) = default;
	Quaternion(Quaternion&& move) = default;
	Quaternion& operator=(const Quaternion& rhs) = default;
	~Quaternion() = default;
	
	using Vector4<float>::getX;
	using Vector4<float>::getY;
	using Vector4<float>::getZ;
	using Vector4<float>::getW;
	
	using Vector4<float>::getXR;
	using Vector4<float>::getYR;
	using Vector4<float>::getZR;
	using Vector4<float>::getWR;
	
	Quaternion conjugate() const;
	Quaternion inverse() const;
	
	using Vector4<float>::length;
	Quaternion normalised() const;
	
	Quaternion operator*(const Quaternion& rhs) const;
	explicit operator Matrix4x4() const;
	
	static Matrix4x4 createModelMatrix(Vector3F position, Vector3F eulerRotation, Vector3F scale);
	static Matrix4x4 createViewMatrix(Vector3F cameraPosition, Vector3F cameraEulerRotation);
};

#endif