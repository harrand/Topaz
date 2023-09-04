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
		trs ret;
		// firstly get scale.
		tz::vec3 m0 = tz::vec4{mat[0]}.swizzle<0, 1, 2>();
		tz::vec3 m1 = tz::vec4{mat[1]}.swizzle<0, 1, 2>();
		tz::vec3 m2 = tz::vec4{mat[2]}.swizzle<0, 1, 2>();
		ret.scale = {m0.length(), m1.length(), m2.length()};
		m0.normalise();
		m1.normalise();
		m2.normalise();
		tz::mat4 rotmat;
		auto [m0x, m0y, m0z] = m0;
		rotmat[0][0] = m0x;
		rotmat[0][1] = m0y;
		rotmat[0][2] = m0z;
		rotmat[0][3] = 0.0f;

		auto [m1x, m1y, m1z] = m0;
		rotmat[1][0] = m1x;
		rotmat[1][1] = m1y;
		rotmat[1][2] = m1z;
		rotmat[1][3] = 0.0f;

		auto [m2x, m2y, m2z] = m0;
		rotmat[2][0] = m2x;
		rotmat[2][1] = m2y;
		rotmat[2][2] = m2z;
		rotmat[2][3] = 0.0f;

		float w = std::sqrt(1.0f - rotmat[0][0] + rotmat[1][1] + rotmat[2][2]) / 2.0f;
		ret.rotate[3] = w;
		ret.rotate[0] = (rotmat[2][1] - rotmat[1][2]) / (w * 4.0f);
		ret.rotate[1] = (rotmat[0][2] - rotmat[2][0]) / (w * 4.0f);
		ret.rotate[2] = (rotmat[1][0] - rotmat[0][1]) / (w * 4.0f);

		ret.translate = tz::vec4{mat[3]}.swizzle<0, 1, 2>();
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