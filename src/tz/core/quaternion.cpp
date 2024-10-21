#include "tz/core/quaternion.hpp"
#include "tz/topaz.hpp"
#include <cmath>

namespace tz
{
	quat::quat(tz::v4f vec): tz::v4f(vec){}

	quat quat::from_axis_angle(tz::v3f axis, float angle)
	{
		return quat
		{tz::v4f{
			axis[0] * std::sin(angle / 2.0f),
			axis[1] * std::sin(angle / 2.0f),
			axis[2] * std::sin(angle / 2.0f),
			std::cos(angle / 2.0f)	
		}}.normalise();
	}

	quat quat::from_euler_angles(tz::v3f euler_angles)
	{
		tz::v3f vx{1.0f, 0.0f, 0.0f}, vy = {0.0f, 1.0f, 0.0f}, vz = {0.0f, 0.0f, 1.0f};
		auto qx = quat::from_axis_angle(vx, euler_angles[0]);
		auto qy = quat::from_axis_angle(vy, euler_angles[1]);
		auto qz = quat::from_axis_angle(vz, euler_angles[2]);
		return ((qx*qy).normalise() * qz).normalise();
	}

	tz::m4f quat::matrix() const
	{
		tz::m4f rot = tz::m4f::iden();
		rot(0, 0) = 1.0f - 2.0f * (this->operator[](1) * this->operator[](1)) - (2 * this->operator[](2) * this->operator[](2));
		rot(1, 0) = (this->operator[](0) * this->operator[](1)) + (2 * this->operator[](2) * this->operator[](3));
		rot(2, 0) = (2 * this->operator[](0) * this->operator[](2)) - (2 * this->operator[](1) * this->operator[](3));
		rot(0, 1) = (2 * this->operator[](0) * this->operator[](1)) - (2 * this->operator[](2) * this->operator[](3));
		rot(1, 1) = 1.0f - 2.0f * (this->operator[](0) * this->operator[](0)) - (2 * this->operator[](2) * this->operator[](2));
		rot(2, 1) = (2 * this->operator[](1) * this->operator[](2)) + (2 * this->operator[](0) * this->operator[](3));
		rot(0, 2) = (2 * this->operator[](0) * this->operator[](2)) + (2 * this->operator[](1) * this->operator[](3));
		rot(1, 2) = (2 * this->operator[](1) * this->operator[](2)) - (2 * this->operator[](0) * this->operator[](3));
		rot(2, 2) = 1.0f - 2.0f * (this->operator[](0) * this->operator[](0)) - (2 * this->operator[](1) * this->operator[](1));

		return rot;
	}

	quat quat::inverse() const
	{
		auto cpy = *this;
		float norm_sq = this->length() * this->length();
		cpy[0] = -cpy[0] * norm_sq;
		cpy[1] = -cpy[1] * norm_sq;
		cpy[2] = -cpy[2] * norm_sq;
		cpy[3] = cpy[3] * norm_sq;
		return *this;
	}

	quat quat::combine(const quat& rhs) const
	{
		return rhs * (*this);
	}

	tz::v3f quat::rotate(tz::v3f pos) const
	{
		tz::v3f me{(*this)[0], (*this)[1], (*this)[2]};
		tz::v3f uv = me.cross(pos);
		tz::v3f uuv = me.cross(uv);
		return pos + ((uv * this->operator[](3)) + uuv) * 2.0f;
	}

	quat quat::normalise() const
	{
		auto cpy = *this;
		float l = this->length();
		if(l == 0.0f)
		{
			// bad
			return quat::iden();
		}
		for(std::size_t i = 0; i < 4; i++)
		{
			cpy[i] /= l;
		}
		return cpy;
	}

	quat quat::slerp(const quat& rhs, float factor) const
	{
		float cos_theta = this->dot(rhs);

		if (cos_theta < 0.0f)
		{
			// If the quaternions are in opposite directions, negate one to take the shortest path
			quat neg_rhs = {{ -rhs[0], -rhs[1], -rhs[2], -rhs[3] }};
			return this->slerp(neg_rhs, factor);
		}

		const float threshold = 0.9995f;
		if (cos_theta > threshold) {
			// Linear factorolation for small angles
			quat result =
			{{
				(*this)[0] + factor * (rhs[0] - (*this)[0]),
				(*this)[1] + factor * (rhs[1] - (*this)[1]),
				(*this)[2] + factor * (rhs[2] - (*this)[2]),
				(*this)[3] + factor * (rhs[3] - (*this)[3])
			}};
			return result.normalise();
		}

		float angle = std::acos(cos_theta);
		float sin_angle = std::sin(angle);
		float t0 = std::sin((1.0f - factor) * angle) / sin_angle;
		float t1 = std::sin(factor * angle) / sin_angle;

		quat result =
		{{
			t0 * (*this)[0] + t1 * rhs[0],
			t0 * (*this)[1] + t1 * rhs[1],
			t0 * (*this)[2] + t1 * rhs[2],
			t0 * (*this)[3] + t1 * rhs[3]
		}};
		
		return result.normalise();
	}

	quat& quat::operator*=(const quat& rhs)
	{
		quat& lhs = *this;
		quat result;

		result[3] = lhs[3] * rhs[3] - lhs[0] * rhs[0] - lhs[1] * rhs[1] - lhs[2] * rhs[2];
		result[0] = lhs[3] * rhs[0] + lhs[0] * rhs[3] + lhs[1] * rhs[2] - lhs[2] * rhs[1];
		result[1] = lhs[3] * rhs[1] - lhs[0] * rhs[2] + lhs[1] * rhs[3] + lhs[2] * rhs[0];
		result[2] = lhs[3] * rhs[2] + lhs[0] * rhs[1] - lhs[1] * rhs[0] + lhs[2] * rhs[3];
		std::swap(result, lhs);
		return *this;
	}
}