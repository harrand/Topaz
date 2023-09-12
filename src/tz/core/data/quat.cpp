#include "tz/core/data/quat.hpp"
#include "tz/core/profile.hpp"

namespace tz
{
	quat::quat(const tz::vec4& vec):
	vec4(vec){}

	quat quat::from_axis_angle(tz::vec3 axis, float angle)
	{
		return quat
		{
			axis[0] * std::sin(angle / 2.0f),
			axis[1] * std::sin(angle / 2.0f),
			axis[2] * std::sin(angle / 2.0f),
			std::cos(angle / 2.0f)	
		}.normalised();
	}

	tz::mat4 quat::matrix() const
	{
		TZ_PROFZONE("Quaternion - Generate Matrix", 0xFF0000AA);
		tz::mat4 rot = tz::mat4::identity();
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

	quat& quat::inverse()
	{
		float norm_sq = this->length() * this->length();
		(*this)[0] = -(*this)[0] * norm_sq;
		(*this)[1] = -(*this)[1] * norm_sq;
		(*this)[2] = -(*this)[2] * norm_sq;
		(*this)[3] = (*this)[3] * norm_sq;
		return *this;
	}

	quat quat::inversed() const
	{
		quat cpy = *this;
		return cpy.inverse();
	}

	void quat::combine(const quat& rhs)
	{
		TZ_PROFZONE("Quaternion - Combine", 0xFF0000AA);
		*this = rhs * (*this);
	}

	quat quat::combined(const quat& rhs) const
	{
		quat cpy = *this;
		cpy.combine(rhs);
		return cpy;
	}

	tz::vec3 quat::rotate(tz::vec3 position) const
	{
		TZ_PROFZONE("Quaternion - Rotate Vec3", 0xFF0000AA);
		tz::vec3 uv = tz::cross(this->swizzle<0, 1, 2>(), position);
		tz::vec3 uuv = tz::cross(this->swizzle<0, 1, 2>(), uv);
		return position + ((uv * this->operator[](3)) + uuv) * 2.0f;
	}

	void quat::normalise()
	{
		TZ_PROFZONE("Quaternion - Normalise", 0xFF0000AA);
		float l = this->length();
		if(l == 0.0f)
		{
			return;
		}
		for(std::size_t i = 0; i < 4; i++)
		{
			(*this)[i] /= l;
		}
	}

	quat quat::normalised() const
	{
		quat cpy = *this;
		cpy.normalise();
		return cpy;
	}

	quat quat::slerp(const quat& rhs, float factor) const
	{
		TZ_PROFZONE("Quaternion - Slerp", 0xFF0000AA);
		float cos_theta = this->dot(rhs);

		if (cos_theta < 0.0f)
		{
			// If the quaternions are in opposite directions, negate one to take the shortest path
			quat neg_rhs = { -rhs[0], -rhs[1], -rhs[2], -rhs[3] };
			return this->slerp(neg_rhs, factor);
		}

		const float threshold = 0.9995f;
		if (cos_theta > threshold) {
			// Linear factorolation for small angles
			quat result =
			{
				(*this)[0] + factor * (rhs[0] - (*this)[0]),
				(*this)[1] + factor * (rhs[1] - (*this)[1]),
				(*this)[2] + factor * (rhs[2] - (*this)[2]),
				(*this)[3] + factor * (rhs[3] - (*this)[3])
			};
			return result.normalised();
		}

		float angle = std::acos(cos_theta);
		float sin_angle = std::sin(angle);
		float t0 = std::sin((1.0f - factor) * angle) / sin_angle;
		float t1 = std::sin(factor * angle) / sin_angle;

		quat result = {
			t0 * (*this)[0] + t1 * rhs[0],
			t0 * (*this)[1] + t1 * rhs[1],
			t0 * (*this)[2] + t1 * rhs[2],
			t0 * (*this)[3] + t1 * rhs[3]
		};
		
		return result.normalised();
	}

	quat& quat::operator*=(const quat& rhs)
	{
		TZ_PROFZONE("Quaternion - Multiply", 0xFF0000AA);
		quat& lhs = *this;
		quat result = quat::zero();

		result[3] = lhs[3] * rhs[3] - lhs[0] * rhs[0] - lhs[1] * rhs[1] - lhs[2] * rhs[2];
		result[0] = lhs[3] * rhs[0] + lhs[0] * rhs[3] + lhs[1] * rhs[2] - lhs[2] * rhs[1];
		result[1] = lhs[3] * rhs[1] - lhs[0] * rhs[2] + lhs[1] * rhs[3] + lhs[2] * rhs[0];
		result[2] = lhs[3] * rhs[2] + lhs[0] * rhs[1] - lhs[1] * rhs[0] + lhs[2] * rhs[3];
		std::swap(result, lhs);
		return *this;
	}

	quat quat::operator*(const quat& rhs) const
	{
		quat cpy = *this;
		return cpy *= rhs;
	}
}