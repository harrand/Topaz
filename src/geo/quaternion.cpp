#include "geo/quaternion.hpp"

namespace tz
{
	Quaternion::Quaternion(): tz::Vec4(){}

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

	/*static*/ void Quaternion::swap(Quaternion& lhs, Quaternion& rhs)
	{
		std::swap(lhs[0], rhs[0]);
		std::swap(lhs[1], rhs[1]);
		std::swap(lhs[2], rhs[2]);
		std::swap(lhs[3], rhs[3]);
	}
}