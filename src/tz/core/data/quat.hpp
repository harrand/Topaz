#ifndef TOPAZ_CORE_DATA_QUAT_HPP
#define TOPAZ_CORE_DATA_QUAT_HPP
#include "tz/core/data/vector.hpp"
#include "tz/core/matrix.hpp"

namespace tz
{
	class quat : public tz::vec4
	{
	public:
		using tz::vec4::vec4;
		quat(const tz::vec4& vec);
		static quat from_axis_angle(tz::vec3 axis, float angle);
		tz::mat4 matrix() const;
		quat& inverse();
		quat inversed() const;
		void combine(const quat& rhs);
		quat combined(const quat& rhs) const;
		tz::vec3 rotate(tz::vec3 position) const;
		void normalise();
		quat normalised() const;
		quat slerp(const quat& rhs, float factor) const;

		quat& operator*=(const quat& rhs);
		quat operator*(const quat& rhs) const;

		static quat zero(){return {0.0f, 0.0f, 0.0f, 0.0f};}
		static quat filled(float f){return {f, f, f, f};}
	};
};

namespace std
{
	template<>
	struct hash<tz::quat> : public hash<tz::vec4>{};
}

#endif // TOPAZ_CORE_DATA_QUAT_HPP