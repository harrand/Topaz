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
		if(mat(3, 0) != 0.0f || mat(3, 1) != 0.0f || mat(3, 2) != 0.0f || mat(3, 3) != 1.0f)
		{
			#if TZ_DEBUG
			mat.debug_print();
			#endif // TZ_DEBUG
			tz::report("Warning: Matrix is not decomposable. Behaviour is unspecified.");
		}

		trs ret;
		// tz::assert(mat.determinant() != 0.0f, "matrix is not decomposable because its determinant is 0.0f");
		ret.translate = tz::vec4{mat[3]}.swizzle<0, 1, 2>();
		ret.scale[0] = std::sqrt(mat(0, 0) * mat(0, 0) + mat(1, 0) * mat(1, 0) + mat(2, 0) * mat(2, 0));
		ret.scale[1] = std::sqrt(mat(0, 1) * mat(0, 1) + mat(1, 1) * mat(1, 1) + mat(2, 1) * mat(2, 1));
		ret.scale[2] = std::sqrt(mat(0, 2) * mat(0, 2) + mat(1, 2) * mat(1, 2) + mat(2, 2) * mat(2, 2));

		// if(mat.determinant() < 0.0f)
		// {
		// ret.scale[0] *= -1.0f;
		//	}

		float invsx = 1.0f / ret.scale[0];
		float invsy = 1.0f / ret.scale[1];
		float invsz = 1.0f / ret.scale[2];
		tz::mat3 r;
		r(0, 0) = mat(0, 0) * invsx;
		r(1, 0) = mat(1, 0) * invsx;
		r(2, 0) = mat(2, 0) * invsx;

		r(0, 1) = mat(0, 1) * invsy;
		r(1, 1) = mat(1, 1) * invsy;
		r(2, 1) = mat(2, 1) * invsy;

		r(0, 2) = mat(0, 2) * invsz;
		r(1, 2) = mat(1, 2) * invsz;
		r(2, 2) = mat(2, 2) * invsz;

		// rotation = rotation from rotation matrix r.
		// mat3 -> quat conversion takes place here.
		const float trace = mat(0, 0) + mat(1, 1) + mat(2, 2);
		if(trace > 0.0f)
		{
			const float s = 0.5f / std::sqrt(trace + 1.0f);
			ret.rotate[3] = 0.25f / s;
			ret.rotate[0] = (mat(2, 1) - mat(1, 2)) * s;
			ret.rotate[1] = (mat(0, 2) - mat(2, 0)) * s;
			ret.rotate[2] = (mat(1, 0) - mat(0, 1)) * s;
		}
		else
		{
			if(mat(0, 0) > mat(1, 1) && mat(0, 0) > mat(2, 2))
			{
				const float s = 2.0f * std::sqrt(1.0f + mat(0, 0) - mat(1, 1) - mat(2, 2));
				ret.rotate[3] = (mat(2, 1) - mat(1, 2)) / s;
				ret.rotate[0] = 0.25f * s;
				ret.rotate[1] = (mat(0, 1) + mat(1, 0)) / s;
				ret.rotate[2] = (mat(0, 2) + mat(2, 0)) / s;
			}
			else if(mat(1, 1) > mat(2, 2))
			{
				const float s = 2.0f * std::sqrt(1.0f + mat(1, 1) - mat(0, 0) - mat(2, 2));
				ret.rotate[3] = (mat(0, 2) - mat(2, 0)) / s;
				ret.rotate[0] = (mat(0, 1) + mat(1, 0)) / s;
				ret.rotate[1] = 0.25f * s;
				ret.rotate[2] = (mat(1, 2) + mat(2, 1)) / s;
			}
			else
			{
				const float s = 2.0f * std::sqrt(1.0f + mat(2, 2) - mat(0, 0) - mat(1, 1));
				ret.rotate[3] = (mat(1, 0) - mat(0, 1)) / s;
				ret.rotate[0] = (mat(0, 2) + mat(2, 0)) / s;
				ret.rotate[1] = (mat(1, 2) + mat(2, 1)) / s;
				ret.rotate[2] = 0.25f * s;
			}
		}
		return ret;
	}

	trs& trs::combine(const trs& t)
	{
		TZ_PROFZONE("TRS - Combine", 0xFF0000AA);
		tz::vec3 rotated = t.rotate.rotate(this->translate);
		tz::vec3 scaled = t.scale * rotated;
		this->translate = t.translate + scaled;
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