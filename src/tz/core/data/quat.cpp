#include "tz/core/data/quat.hpp"

namespace tz
{
	quat::quat(const tz::vec4& vec):
	vec4(vec){}

	tz::mat4 quat::matrix() const
	{
		tz::mat4 rot = tz::mat4::identity();
		rot(0, 0) = 1.0f - (2 * this->operator[](1) * this->operator[](1)) - (2 * this->operator[](2) * this->operator[](2));
		rot(1, 0) = (2 * this->operator[](0) * this->operator[](1)) + (2 * this->operator[](2) * this->operator[](3));
		rot(2, 0) = (2 * this->operator[](0) * this->operator[](2)) - (2 * this->operator[](1) * this->operator[](3));
		rot(0, 1) = (2 * this->operator[](0) * this->operator[](1)) - (2 * this->operator[](2) * this->operator[](3));
		rot(1, 1) = 1.0f - (2 * this->operator[](0) * this->operator[](0)) - (2 * this->operator[](2) * this->operator[](2));
		rot(2, 1) = (2 * this->operator[](1) * this->operator[](2)) + (2 * this->operator[](0) * this->operator[](3));
		rot(0, 2) = (2 * this->operator[](0) * this->operator[](2)) + (2 * this->operator[](1) * this->operator[](3));
		rot(1, 2) = (2 * this->operator[](1) * this->operator[](2)) - (2 * this->operator[](0) * this->operator[](3));
		rot(2, 2) = 1.0f - (2 * this->operator[](0) * this->operator[](0)) - (2 * this->operator[](1) * this->operator[](1));

		return rot;
	}

	void quat::combine(const quat& rhs)
	{
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
		tz::vec3 uv = tz::cross(this->swizzle<0, 1, 2>(), position);
		tz::vec3 uuv = tz::cross(this->swizzle<0, 1, 2>(), uv);
		return position + ((uv * this->operator[](3)) + uuv) * 2.0f;
	}

	void quat::normalise()
	{
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
		quat& lhs = *this;
		quat result = quat::zero();

		result[0] = rhs[3] * lhs[0] + rhs[0] * lhs[3] + rhs[1] * lhs[2] - rhs[2] * lhs[1];
		result[1] = rhs[3] * lhs[1] + rhs[1] * lhs[3] + rhs[2] * lhs[0] - rhs[0] * lhs[2];
		result[2] = rhs[3] * lhs[2] + rhs[2] * lhs[3] + rhs[0] * lhs[1] - rhs[1] * lhs[0];
		result[3] = rhs[3] * lhs[3] - rhs[0] * lhs[0] - rhs[1] * lhs[1] - rhs[2] * lhs[2];
		std::swap(result, lhs);
		return *this;
	}

	quat quat::operator*(const quat& rhs) const
	{
		quat cpy = *this;
		return cpy *= rhs;
	}
}