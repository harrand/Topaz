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
		for(std::size_t i = 0; i < this->scale.data().size(); i++)
		{
			this->scale[i] *= t.scale[i];
		}
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