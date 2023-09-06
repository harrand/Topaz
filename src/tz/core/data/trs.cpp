#include "tz/core/data/trs.hpp"
#include "tz/core/matrix_transform.hpp"

namespace tz
{
	trs trs::lerp(const trs& rhs, float factor) const
	{
		TZ_PROFZONE("TRS - Lerp", 0xFF0000AA);
		trs ret;
		ret.translate = this->translate + ((rhs.translate - this->translate) * factor);
		ret.rotate = this->rotate.slerp(rhs.rotate, factor);
		ret.scale = this->scale + ((rhs.scale - this->scale) * factor);
		return ret;
	}

	tz::mat4 trs::matrix() const
	{
		TZ_PROFZONE("TRS - Generate Matrix", 0xFF0000AA);
		return tz::translate(this->translate) * this->rotate.matrix() * tz::scale(this->scale);
	}

	trs trs::from_matrix(tz::mat4 mat)
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
		ret.translate = tz::vec4{mat[3]}.swizzle<0, 1, 2>();
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

	trs& trs::combine(const trs& t)
	{
		TZ_PROFZONE("TRS - Combine", 0xFF0000AA);
		//this->translate += t.translate;
		this->translate = t.translate + t.scale * (t.rotate.rotate(this->translate));
		// are we sure we don't just add these aswell?
		this->rotate.combine(t.rotate);
		this->rotate.normalise();
		// possibly ignore scale entirely?
		this->scale *= t.scale;
		return *this;
	}

	trs trs::combined(const trs& t) const
	{
		trs cpy = *this;
		return cpy.combine(t);
	}

	void trs::dbgui()
	{
		#if TZ_DEBUG
		ImGui::SliderFloat3("Position", this->translate.data().data(), -this->dbgui_slider_scale[0], this->dbgui_slider_scale[0]);
		ImGui::SameLine();
		ImGui::InputFloat("Position Limits", &this->dbgui_slider_scale[0], 0.1f, 1.0f);
		if(ImGui::SliderFloat4("Quaternion", this->rotate.data().data(), -3.14159f, 3.14159f))
		{
			this->rotate.normalise();
		}
		ImGui::SliderFloat3("Scale", this->scale.data().data(), -this->dbgui_slider_scale[1], this->dbgui_slider_scale[1]);
		ImGui::SameLine();
		ImGui::InputFloat("Scale Limits", &this->dbgui_slider_scale[1], 0.1f, 1.0f);
		#endif // TZ_DEBUG
	}
}