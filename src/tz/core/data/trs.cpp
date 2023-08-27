#include "tz/core/data/trs.hpp"
#include "tz/core/matrix_transform.hpp"

namespace tz
{
	trs trs::lerp(const trs& rhs, float factor) const
	{
		trs ret;
		ret.translate = this->translate + ((rhs.translate - this->translate) * factor);
		ret.rotate = this->rotate.slerp(rhs.rotate, factor);
		ret.scale = this->scale + ((rhs.scale - this->scale) * factor);
		return ret;
	}

	tz::mat4 trs::matrix() const
	{
		return tz::translate(this->translate) * this->rotate.matrix() * tz::scale(this->scale);
	}

	trs& trs::combine(const trs& t)
	{
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
}