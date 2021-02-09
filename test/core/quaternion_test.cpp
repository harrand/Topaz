//
// Created by Harrand on 06/06/2020.
//

#include "test_framework.hpp"
#include "core/tz.hpp"
#include "core/quaternion.hpp"
#include "core/matrix_transform.hpp"
#include <string>
#include <cstring>

TZ_TEST_BEGIN(default_ctor_no_transform)
	tz::Quaternion rot{{{0.0f, 3.14159f, 0.0f}}};
	tz::Quaternion transformed = rot * tz::Quaternion{};
	topaz_expect(rot == transformed, "Default constructed tz::Quaternion is performing transformation!");
TZ_TEST_END

TZ_TEST_BEGIN(matrix)
	// Rotate a position about the origin about the y-axis.
	tz::Vec4 subject{{0.0f, 0.0f, 1.0f, 1.0f}};
	tz::Quaternion quat = tz::Quaternion::from_axis({{{0.0f, 1.0f, 0.0f}}, 3.14159f / 2.0f});
	tz::Mat4 mat = tz::rotate({{0.0f, -3.14159f / 2.0f, 0.0f}});

	tz::Vec4 result1 = quat.to_matrix() * subject;
	tz::Vec4 result2 = mat * subject;
	topaz_expect(result1 == result2, "Equivalent matrix and quaternions produced different transformations. Result1 = {", result1[0], ", ", result1[1], ", ", result1[2], ", ", result1[3], "}, Result2 = {", result2[0], ", ", result2[1], ", ", result2[2], ", ", result2[3], "}");
TZ_TEST_END

TZ_TEST_BEGIN(euler_quat_euler)
	constexpr float pi = 3.14159f;
	tz::Vec3 rot{{0, pi, pi}};
	tz::Quaternion quat = tz::Quaternion::from_eulers(rot);
	tz::Vec3 after = quat.to_eulers();

	// rot may not be equal to after, but they should perform the same rotation.
	tz::Vec4 subject{{0.0f, 0.0f, 1.0f, 1.0f}};
	tz::Mat4 rot_a = tz::rotate(rot);
	tz::Mat4 rot_b = tz::rotate(after);
	{
		tz::Vec4 rx = rot_a * subject;
		tz::Vec4 ry = rot_b * subject;
		topaz_expect((rx-ry).length() < 0.01f, "Euler->Quat->Euler: Euler0 is not the same rotation as Euler1. Euler0 = {", rx[0], ", ", rx[1], ", ", rx[2], "}. Euler1 = {", ry[0], ", ", ry[1], ", ", ry[2], "}");
	}
TZ_TEST_END

int main()
{
	tz::test::Unit quat;
	quat.add(default_ctor_no_transform());
	quat.add(matrix());
	quat.add(euler_quat_euler());
	return quat.result();
}