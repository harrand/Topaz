#include "quaternion.hpp"

Quaternion::Quaternion(float x, float y, float z, float w): Vector4F(x, y, z, w){}

Quaternion::Quaternion(const Vector3F& euler_rotation)
{
	Quaternion qx(Vector3F(1, 0, 0), euler_rotation.getX());
	Quaternion qy(Vector3F(0, 1, 0), euler_rotation.getY());
	Quaternion qz(Vector3F(0, 0, 1), euler_rotation.getZ());
	Quaternion q = (qx * qy) * qz;
	Quaternion(q.getX(), q.getY(), q.getZ(), q.getW());
}

Quaternion::Quaternion(const Vector3F& rotation_axis, float angle)
{
	Vector3F axis = rotation_axis.normalised();
	this->x = axis.getX() * sin(angle / 2);
	this->y = axis.getY() * sin(angle / 2);
	this->z = axis.getZ() * sin(angle / 2);
	this->w = cos(angle / 2);
}

Quaternion::Quaternion(const Matrix4x4& rotation)
{
	float trace = 1 + rotation.getRowX().getX() + rotation.getRowY().getY() + rotation.getRowZ().getZ();
	float s;
	if(trace > 0.0001f)
	{
		s = sqrt(trace) * 2;
		this->x = (rotation.getRowZ().getY() - rotation.getRowY().getZ()) / s;
		this->y = (rotation.getRowX().getZ() - rotation.getRowZ().getX()) / s;
		this->z = (rotation.getRowY().getX() - rotation.getRowX().getY()) / s;
		this->w = s / 4;
	}
	else if(trace > -0.0001f)
	{
		if(rotation.getRowX().getX() > rotation.getRowY().getY() && rotation.getRowX().getX() > rotation.getRowZ().getZ())
		{
			s = sqrt(1.0 + rotation.getRowX().getX() - rotation.getRowY().getY() - rotation.getRowZ().getZ()) * 2;
			this->x = s / 4;
			this->y = (rotation.getRowY().getX() + rotation.getRowX().getY()) / s;
			this->z = (rotation.getRowX().getZ() + rotation.getRowZ().getX()) / s;
			this->w = (rotation.getRowZ().getY() - rotation.getRowY().getZ()) / s;
		}
		else if(rotation.getRowY().getY() > rotation.getRowZ().getZ())
		{
			s = sqrt(1.0 + rotation.getRowY().getY() - rotation.getRowX().getX() - rotation.getRowZ().getZ()) * 2;
			this->x = (rotation.getRowY().getX() + rotation.getRowX().getY()) / s;
			this->y = s / 4;
			this->z = (rotation.getRowZ().getY() + rotation.getRowY().getZ()) / s;
			this->w = (rotation.getRowX().getZ() - rotation.getRowZ().getX()) / s;
		}
		else
		{
			s = sqrt(1.0 + rotation.getRowZ().getZ() - rotation.getRowX().getX() - rotation.getRowY().getY()) * 2;
			this->x = (rotation.getRowX().getZ() + rotation.getRowZ().getX()) / s;
			this->y = (rotation.getRowZ().getY() + rotation.getRowY().getZ()) / s;
			this->z = s / 4;
			this->w = (rotation.getRowY().getX() - rotation.getRowX().getY()) / s;
		}
	}
	else
	{
		// Trace is basically 0 and hence invalid
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->w = 0;
	}
}

Quaternion Quaternion::conjugate() const
{
	return Quaternion(-this->x, -this->y, -this->z, this->w);
}

Quaternion Quaternion::inverse() const
{
	return this->normalised().conjugate();
}

Quaternion Quaternion::normalised() const
{
	return Quaternion(this->x / this->length(), this->y / this->length(), this->z / this->length(), this->w / this->length());
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const
{
	float x = this->w * rhs.getX() + this->x * rhs.getW() + this->y * rhs.getZ() - this->z * rhs.getY();
	float y = this->w * rhs.getY() + this->y * rhs.getW() + this->z * rhs.getX() - this->x * rhs.getZ();
	float z = this->w * rhs.getZ() + this->z * rhs.getW() + this->x * rhs.getY() - this->y * rhs.getX();
	float w = this->w * rhs.getW() - this->x * rhs.getX() - this->y * rhs.getY() - this->z * rhs.getZ();
	return Quaternion(x, y, z, w).normalised();
}

Quaternion::operator Matrix4x4() const
{
	Vector4F x(1.0f - 2.0f * (this->y * this->y + this->z * this->z), 2.0f * (this->x * this->y + this->z * this->w), 2.0f * (this->x * this->z - this->y * this->w), 0.0f);
	Vector4F y(2.0f * (this->x * this->y - this->z * this->w), 1.0f - 2.0f * (this->x * this->x + this->z * this->z), 2.0f * (this->y * this->z + this->x * this->w), 0.0f);
	Vector4F z(2.0f * (this->x * this->z + this->y * this->w), 2.0f * (this->y * this->z - this->x * this->w), 1.0f - 2.0f * (this->x * this->x + this->y * this->y), 0.0f);
	Vector4F w(0.0f, 0.0f, 0.0f, 1.0f);
	return Matrix4x4(x, y, z, w);
}

Matrix4x4 Quaternion::createModelMatrix(Vector3F position, Vector3F eulerRotation, Vector3F scale)
{
	return matrixtransformations::createTranslationMatrix(position) * static_cast<Matrix4x4>(Quaternion(eulerRotation).normalised()) * matrixtransformations::createScalingMatrix(scale);
}

Matrix4x4 Quaternion::createViewMatrix(Vector3F cameraPosition, Vector3F cameraEulerRotation)
{
	return Quaternion::createModelMatrix(cameraPosition, cameraEulerRotation, Vector3F(1, 1, 1)).inverse();
}