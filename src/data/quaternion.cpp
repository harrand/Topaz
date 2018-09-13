#include "data/quaternion.hpp"

Quaternion::Quaternion(Vector3F rotation_axis, float angle): Vector4F(rotation_axis.x * std::sin(angle / 2.0f), rotation_axis.y * std::sin(angle / 2.0f), rotation_axis.z * std::sin(angle / 2.0f), std::cos(angle / 2.0f)){}

Quaternion::Quaternion(Vector3F euler_rotation)
{
	const float sin_pitch(std::sin(euler_rotation.x / 2.0f));
	const float cos_pitch(std::cos(euler_rotation.x / 2.0f));
	const float sin_yaw(std::sin(euler_rotation.y / 2.0f));
	const float cos_yaw(std::cos(euler_rotation.y / 2.0f));
	const float sin_roll(std::sin(euler_rotation.z / 2.0f));
	const float cos_roll(std::cos(euler_rotation.z / 2.0f));
	const float cos_pitch_cos_yaw(cos_pitch*cos_yaw);
	const float sin_pitch_sin_yaw(sin_pitch*sin_yaw);

	this->x = sin_roll * cos_pitch_cos_yaw - cos_roll * sin_pitch_sin_yaw;
	this->y = cos_roll * sin_pitch * cos_yaw + sin_roll * cos_pitch * sin_yaw;
	this->z = cos_roll * cos_pitch * sin_yaw - sin_roll * sin_pitch * cos_yaw;
	this->w = cos_roll * cos_pitch_cos_yaw + sin_roll * sin_pitch_sin_yaw;
}

Quaternion::Quaternion(Matrix4x4 rotational_matrix)
{
	// takes a row-major matrix.
	float trace = 1 + rotational_matrix.x.x + rotational_matrix.y.y + rotational_matrix.z.z;
	float s;
	if(trace > 0.000001f)
	{
		// perform an instant calculation.
		s = std::sqrt(trace) * 2.0f;
		this->x = (rotational_matrix.y.z - rotational_matrix.z.y) / s;
		this->y = (rotational_matrix.z.x - rotational_matrix.x.z) / s;
		this->z = (rotational_matrix.x.y - rotational_matrix.y.x) / s;
		this->w = 0.25f * s;
	}
	else
	{
		if(rotational_matrix.x.x > rotational_matrix.y.y && rotational_matrix.x.x > rotational_matrix.z.z)
		{
			s = std::sqrt(1.0f + rotational_matrix.x.x - rotational_matrix.y.y - rotational_matrix.z.z) * 2.0f;
			this->x = 0.25f * s;
			this->y = (rotational_matrix.x.y + rotational_matrix.y.x) / s;
			this->z = (rotational_matrix.z.x + rotational_matrix.x.z) / s;
			this->w = (rotational_matrix.y.z + rotational_matrix.z.y) / s;
		}
		else if(rotational_matrix.y.y > rotational_matrix.z.z)
		{
			s = std::sqrt(1.0f + rotational_matrix.y.y - rotational_matrix.x.x - rotational_matrix.z.z) * 2.0f;
			this->x = (rotational_matrix.x.y + rotational_matrix.y.x) / s;
			this->y = 0.25f * s;
			this->z = (rotational_matrix.y.z + rotational_matrix.z.y) / s;
			this->w = (rotational_matrix.z.x - rotational_matrix.x.z) / s;
		}
		else
		{
			s = std::sqrt(1.0f + rotational_matrix.z.z - rotational_matrix.x.x - rotational_matrix.y.y) * 2.0f;
			this->x = (rotational_matrix.z.x + rotational_matrix.x.z) / s;
			this->y = (rotational_matrix.y.z + rotational_matrix.z.y) / s;
			this->z = 0.25f * s;
			this->w = (rotational_matrix.x.y - rotational_matrix.y.x) / s;
		}
	}
}

Quaternion::Quaternion(Vector4F xyzw): Vector4F(xyzw){}

float Quaternion::get_angle() const
{
	return std::acos(this->w) * 2.0f;
}

Vector3F Quaternion::get_rotation_axis() const
{
	float sin_a = std::sqrt(1.0f - std::pow(this->w, 2));
	if(std::abs(sin_a) < 0.0005f)
		sin_a = 1.0f;
	return {this->x / sin_a, this->y / sin_a, this->z / sin_a};
}

Matrix4x4 Quaternion::to_matrix() const
{
	return Matrix4x4(
	Vector4F(1 - (2 * std::pow(this->y, 2) + 2 * std::pow(this->z, 2)), 2 * this->x * this->y + 2 * this->z * this->w, 2 * this->x * this->z - 2 * this->y * this->w, 0),
	Vector4F(2 * this->x * this->y - 2 * this->z * this->w, 1 - (2 * std::pow(this->x, 2) + 2 * std::pow(this->z, 2)), 2 * this->y * this->z + 2 * this->x * this->w, 0),
	Vector4F(2 * this->x * this->z + 2 * this->y * this->w, 2 * this->y * this->z - 2 * this->x * this->w, 1 - (2 * std::pow(this->x, 2) + 2 * std::pow(this->y, 2)), 0.0f),
	Vector4F(0.0f, 0.0f, 0.0f, 1.0f)).transposed();
}

Quaternion Quaternion::normalised() const
{
	return {(*this) / this->length()};
}

Quaternion Quaternion::inverse() const
{
	return -(this->normalised());
}

Quaternion Quaternion::operator-() const
{
	Vector4F copy = *this;
	copy.x = -copy.x;
	copy.y = -copy.y;
	copy.z = -copy.z;
	return {copy};
}

Matrix4x4 Quaternion::operator()() const
{
	return this->to_matrix();
}

Quaternion::operator Matrix4x4() const
{
	return this->to_matrix();
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const
{
	return {Vector4F(this->w * rhs.x + this->x + rhs.w + this->y * rhs.z - this->z * rhs.y, this->w * rhs.y + this->y * rhs.w + this->z * rhs.x - this->x * rhs.z, this->w * rhs.z + this->z * rhs.w + this->x * rhs.y - this->y * rhs.x, this->w * rhs.w - this->x * rhs.x - this->y * rhs.y - this->z * rhs.z)};
}

Quaternion Quaternion::operator*(float scalar) const
{
	if(this->length() == 0)
		return {};
	return {Vector4F(this->x * scalar, this->y * scalar, this->z * scalar, this->w * scalar)};
}

Quaternion Quaternion::operator/(float scalar) const
{
    return (*this) * (1.0f / scalar);
}

Vector4F Quaternion::operator*(const Vector3F& vector) const
{
	return (*this) * Quaternion(vector, 0.0f);
}