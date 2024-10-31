#include "tz/core/trs.hpp"
#include "tz/topaz.hpp"

namespace tz
{
	tz::m4f matrix_translate(tz::v3f translate)
	{
		auto m = tz::m4f::iden();
		m(0, 3) = translate[0];
		m(1, 3) = translate[1];
		m(2, 3) = translate[2];
		return m;
	}

	tz::m4f matrix_scale(tz::v3f scale)
	{
		auto m = tz::m4f::iden();
		m(0, 0) = scale[0];
		m(1, 1) = scale[1];
		m(2, 2) = scale[2];
		return m;
	}

	trs trs::lerp(const trs& rhs, float factor) const
	{
		trs ret;
		ret.translate = this->translate + (rhs.translate - this->translate) * factor;
		ret.rotate = this->rotate.slerp(rhs.rotate, factor);
		ret.scale = this->scale + ((rhs.scale - this->scale) * factor);
		return ret;
	}

	tz::m4f trs::matrix() const
	{
		return matrix_scale(this->scale) * matrix_translate(this->translate) * this->rotate.matrix();
	}

	trs trs::from_matrix(tz::m4f mat)
	{
		// decompose matrix -> trs using:https://github.com/KhronosGroup/glTF-Validator/issues/33 
		//If M[3] != 0.0 || M[7] != 0.0 || M[11] != 0.0 || M[15] != 1.0,
		// matrix is indecomposable, exit.

		/*
		0 4 8  12
		1 5 9  13
		2 6 10 14
		3 7 11 15
		*/
		trs ret;
		ret.translate = {mat(3, 0), mat(3, 1), mat(3, 2)};
		ret.scale[0] = std::sqrt(mat(0, 0) * mat(0, 0) + mat(1, 0) * mat(1, 0) + mat(2, 0) * mat(2, 0));
		ret.scale[1] = std::sqrt(mat(0, 1) * mat(0, 1) + mat(1, 1) * mat(1, 1) + mat(2, 1) * mat(2, 1));
		ret.scale[2] = std::sqrt(mat(0, 2) * mat(0, 2) + mat(1, 2) * mat(1, 2) + mat(2, 2) * mat(2, 2));

		float isx = 1.0f / ret.scale[0];
		float isy = 1.0f / ret.scale[1];
		float isz = 1.0f / ret.scale[2];

		// remove scaling from matrix
		mat(0, 0) *= isx; mat(1, 0) *= isx; mat(1, 0) *= isx;
		mat(0, 1) *= isy; mat(1, 1) *= isy; mat(2, 1) *= isy;
		mat(0, 2) *= isz; mat(1, 2) *= isz; mat(2, 2) *= isz;

		// Construct the quaternion. This algo is copied from here:
		// https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/christian.htm.
		ret.rotate[3] = std::max(0.0f, 1.0f + mat(0, 0) + mat(1, 1) + mat(2, 2));
		ret.rotate[0] = std::max(0.0f, 1.0f + mat(0, 0) - mat(1, 1) - mat(2, 2));
		ret.rotate[1] = std::max(0.0f, 1.0f - mat(0, 0) + mat(1, 1) - mat(2, 2));
		ret.rotate[2] = std::max(0.0f, 1.0f - mat(0, 0) - mat(1, 1) - mat(2, 2));
		for(std::size_t i = 0; i < 4; i++)
		{
			ret.rotate[i] = std::sqrt(ret.rotate[i] * 0.5f);
		}
		ret.rotate[0] = std::copysignf(ret.rotate[0], mat(1, 2) - mat(2, 1));
		ret.rotate[1] = std::copysignf(ret.rotate[1], mat(2, 0) - mat(0, 2));
		ret.rotate[2] = std::copysignf(ret.rotate[2], mat(0, 1) - mat(1, 0));
		return ret;
	}

	
}