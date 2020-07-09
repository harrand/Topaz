#include "geo/quaternion.hpp"
#include "algo/math.hpp"

namespace tz
{
	Quaternion::Quaternion(): Quaternion{{{0.0f, 1.0f, 0.0f}}, 0.0f}{}

	Quaternion::Quaternion(tz::Vec3 axis, float angle): tz::Vec4{{}}
	{
		axis.normalise();
		const float sin_a = std::sin(angle / 2.0f);
		const float cos_a = std::cos(angle / 2.0f);
		Quaternion& q = *this;
		q[0] = axis[0] * sin_a;
		q[1] = axis[1] * sin_a;
		q[2] = axis[2] * sin_a;
		q[3] = cos_a;
	}

	Quaternion::Quaternion(tz::Vec3 euler_angles): Quaternion{}
	{
		// Simply construct 3 quaternions in each axis, and combine rotations.
		Quaternion x{{{1.0f, 0.0f, 0.0f}}, euler_angles[0]};
		Quaternion y{{{0.0f, 1.0f, 0.0f}}, euler_angles[1]};
		Quaternion z{{{0.0f, 0.0f, 1.0f}}, euler_angles[2]};
		*this *= (x * y * z);
		this->normalise();
	}

	Quaternion Quaternion::conjugated() const
	{
		Quaternion cpy = *this;
		cpy[0] *= -1.0f;
		cpy[1] *= -1.0f;
		cpy[2] *= -1.0f;
		return cpy;
	}

	Quaternion Quaternion::inversed() const
	{
		if(this->length() == 1)
		{
			return this->conjugated();
		}
		else
		{
			// Not normalised. Not quite the same.
			Quaternion q = this->conjugated();
			static_cast<tz::Vec4&>(q) *= (1.0f / this->length());
			return q;
		}	
	}

	void Quaternion::normalise()
	{
		tz::Vec4::normalise();
	}

	Quaternion Quaternion::normalised() const
	{
		Quaternion cpy = *this;
		cpy.normalise();
		return cpy;
	}

	Quaternion::operator tz::Mat4() const
	{
		// http://www.opengl-tutorial.org/assets/faq_quaternions/index.html#Q54
		Quaternion t = this->normalised();
		tz::Mat4 mat = tz::Mat4::identity();
		// 1 - (2Y² + 2Z²)
		mat(0, 0) = 1.0f - ((2.0f * t[1] * t[1]) + (2.0f * t[2] * t[2]));
		// 2XY + 2ZW
		mat(0, 1) = (2.0f * t[0] * t[1]) + (2.0f * t[2] * t[3]);
		// 2XZ - 2YW
		mat(0, 2) = (2.0f * t[0] * t[2]) - (2.0f * t[1] * t[3]);

		// 2XY - 2ZW
		mat(1, 0) = (2.0f * t[0] * t[1]) - (2.0f * t[2] * t[3]);
		// 1 - (2X² + 2Z²)
		mat(1, 1) = 1.0f - ((2.0f * t[0] * t[0]) + (2.0f * t[2] * t[2]));
		// 2YZ + 2XW
		mat(1, 2) = (2.0f * t[1] * t[2]) + (2.0f * t[0] * t[3]);
		
		// 2XZ + 2YW
		mat(2, 0) = (2.0f * t[0] * t[2]) + (2.0f * t[1] * t[3]);
		// 2YZ - 2XW
		mat(2, 1) = (2.0f * t[1] * t[2]) - (2.0f * t[0] * t[3]);
		// 1 - (2X² + 2Y²)
		mat(2, 2) = 1.0f - ((2.0f * t[0] * t[0]) + (2.0f * t[1] * t[1]));
		return mat;
	}

	Quaternion& Quaternion::operator*=(const Quaternion& rhs)
	{
		Quaternion res = *this * rhs;
		Quaternion::swap(*this, res);
		return *this;
	}

	Quaternion Quaternion::operator*(const Quaternion& rhs) const
	{
		const Quaternion& lhs = *this;
		Quaternion cpy;
		// x = w1x2 + x1w2 + y1z2 - z1y2
		cpy[0] = (lhs[3] * rhs[0]) + (lhs[0] * rhs[3]) + (lhs[1] * rhs[2]) - (lhs[2] * rhs[1]);
		// y = w1y2 + y1w2 + z1x2 - x1z2
		cpy[1] = (lhs[3] * rhs[1]) + (lhs[1] * rhs[3]) + (lhs[2] * rhs[0]) - (lhs[0] * rhs[2]); 
		// z = w1z2 + z1w2 + x1y2 - y1x2
		cpy[2] = (lhs[3] * rhs[2]) + (lhs[2] * rhs[3]) + (lhs[0] * rhs[1]) - (lhs[1] * rhs[0]);
		// w = w1w2 - x1x2 - y1y2 - z1z2
		cpy[3] = (lhs[3] * rhs[3]) - (lhs[0] * rhs[0]) - (lhs[1] * rhs[1]) - (lhs[2] * rhs[2]);
		return cpy;
	}

	bool Quaternion::operator==(const Quaternion& rhs) const
	{
		return std::abs(this->dot(rhs)) > (1.0f - std::numeric_limits<float>::epsilon());
	}

	Quaternion Quaternion::from_axis(AxisAndAngle rotation)
	{
		// We already have a ctor for this, just use that.
		return {rotation.axis, rotation.angle};
	}

	AxisAndAngle Quaternion::to_axis() const
	{
		Quaternion q = this->normalised();
		
		AxisAndAngle ret;
		float cos_a = q[3];
		ret.angle = std::acos(cos_a) * 2.0f;
		float sin_a = std::sqrt(1.0f - cos_a * cos_a);
		if(std::fabs(sin_a) < 0.0005f)
		{
			sin_a = 1;
		}
		
		for(std::size_t i = 0; i < 3; i++)
		{
			ret.axis[i] = q[i] / sin_a;
		}

		return ret;
	}

	Quaternion Quaternion::from_eulers(tz::Vec3 euler_angles)
	{
		// Also already have a ctor for this.
		return {euler_angles};
	}

	tz::Vec3 Quaternion::to_eulers() const
	{
		// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#QuaterniontoEulerAnglesConversion
		float qx = (*this)[0]; float qy = (*this)[1]; float qz = (*this)[2]; float qw = (*this)[3];
		float sinr_cosp = 2.0f * (qw * qx + qy * qz);
		float cosr_cosp = 1.0f - 2.0f * (qx * qx + qy * qy);
		// roll
		float x = std::atan2(sinr_cosp, cosr_cosp);

		// pitch
		float sinp = 2.0f * (qw * qy - qz * qx);
		float y;
		if(std::abs(sinp) >= 1)
		{
			y = std::copysignf(3.14159 / 2.0f, sinp);
		}
		else
		{
			y = std::asin(sinp);
		}

		// yaw
		float siny_cosp = 2.0f * (qw * qz + qx * qy);
		float cosy_cosp = 1.0f - 2.0f * (qy * qy + qz * qz);
		float z = std::atan2(siny_cosp, cosy_cosp);

		return {{x, y, z}};
	}

	Quaternion Quaternion::from_matrix(tz::Mat4 rotation)
	{
		// http://www.opengl-tutorial.org/assets/faq_quaternions/index.html#Q55
		float trace = 1.0f + rotation(0, 0) + rotation(1, 1) + rotation(2, 2);

		constexpr float eps = std::numeric_limits<float>::epsilon();
		float s, x, y, z, w;
		if(trace >= eps)
		{
			s = std::sqrt(trace) / 2.0f;
			x = (rotation(1, 2) - rotation(2, 1)) / s;
			y = (rotation(2, 0) - rotation(0, 2)) / s;
			z = (rotation(0, 1) - rotation(1, 0)) / s;
			w = 0.25f * s;
		}

		if(trace < eps && trace >= -eps)
		{
			if(rotation(0, 0) > rotation(1, 1) && rotation(0, 0) > rotation(2, 2))
			{
				s = std::sqrt(1.0f + rotation(1, 1) - rotation(0, 0) - rotation(2, 2)) * 2.0f;
				x = 0.25f * s;
				y = (rotation(0, 1) + rotation(1, 0)) / s;
				z = (rotation(2, 0) + rotation(0, 2)) / s;
				w = (rotation(1, 2) - rotation(2, 1)) / s;
			}
			else if(rotation(1, 1) > rotation(2, 2))
			{
				s = std::sqrt(1.0f + rotation(1, 1) - rotation(0, 0) - rotation(2, 2)) * 2.0f;
				x = (rotation(0, 1) + rotation(1, 0)) / s;
				y = 0.25f * s;
				z = (rotation(1, 2) + rotation(2, 1)) / s;
				w = (rotation(2, 0) - rotation(0, 2)) / s;
			}
			else
			{
				s = std::sqrt(1.0f + rotation(2, 2) - rotation(0, 0) - rotation(1, 1)) * 2.0f;
				x = (rotation(2, 0) + rotation(0, 2)) / s;
				y = (rotation(1, 2) + rotation(2, 1)) / s;
				z = 0.25f * s;
				w = (rotation(0, 1) - rotation(1, 0)) / s;
			}
		}
		return {x, y, z, w};
	}

	tz::Mat4 Quaternion::to_matrix() const
	{
		return static_cast<tz::Mat4>(*this);
	}

	Quaternion::Quaternion(float x, float y, float z, float w): tz::Vec4{{{x, y, z, w}}}{}

	/*static*/ void Quaternion::swap(Quaternion& lhs, Quaternion& rhs)
	{
		std::swap(lhs[0], rhs[0]);
		std::swap(lhs[1], rhs[1]);
		std::swap(lhs[2], rhs[2]);
		std::swap(lhs[3], rhs[3]);
	}
}