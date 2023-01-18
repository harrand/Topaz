#include "tz/core/matrix_transform.hpp"
#include "hdk/profile.hpp"
#include <cmath>

namespace tz
{
	mat4 translate(hdk::vec3 position)
	{
		HDK_PROFZONE("Matrix Translate", 0xFF0000AA);
		mat4 m = mat4::identity();
		m(0, 3) = position[0];
		m(1, 3) = position[1];
		m(2, 3) = position[2];
		return m;
	}

	// http://www.opengl-tutorial.org/assets/faq_quaternions/index.html#Q28
	/*
	Q28. How do I generate a rotation matrix in the X-axis?
		-------------------------------------------------------

		Use the 4x4 matrix:

				|  1  0       0       0 |
			M = |  0  cos(A) -sin(A)  0 |
				|  0  sin(A)  cos(A)  0 |
				|  0  0       0       1 |
	*/
	mat4 rotate_x(float angle)
	{
		mat4 m = mat4::identity();
		m(1, 1) = std::cos(angle);
		m(1, 2) = -std::sin(angle);
		m(2, 1) = std::sin(angle);
		m(2, 2) = std::cos(angle);
		return m;
	}

	/*
		Q29. How do I generate a rotation matrix in the Y-axis?
		-------------------------------------------------------

		Use the 4x4 matrix:

				|  cos(A)  0   sin(A)  0 |
			M = |  0       1   0       0 |
				| -sin(A)  0   cos(A)  0 |
				|  0       0   0       1 |
	*/

	mat4 rotate_y(float angle)
	{
		mat4 m = mat4::identity();
		m(0, 0) = std::cos(angle);
		m(0, 2) = std::sin(angle);
		m(2, 0) = -std::sin(angle);
		m(2, 2) = std::cos(angle);
		return m;
	}

	/*
		Q30. How do I generate a rotation matrix in the Z-axis?
		-------------------------------------------------------

		Use the 4x4 matrix:

				|  cos(A)  -sin(A)   0   0 |
			M = |  sin(A)   cos(A)   0   0 |
				|  0        0        1   0 |
				|  0        0        0   1 |
	 */
	mat4 rotate_z(float angle)
	{
		mat4 m = mat4::identity();
		m(0, 0) = std::cos(angle);
		m(0, 1) = -std::sin(angle);
		m(1, 0) = std::sin(angle);
		m(1, 1) = std::cos(angle);
		return m;
	}

	mat4 rotate(hdk::vec3 rotation)
	{
		HDK_PROFZONE("Matrix Rotate", 0xFF0000AA);
		mat4 x = rotate_x(rotation[0]);
		mat4 y = rotate_y(rotation[1]);
		mat4 z = rotate_z(rotation[2]);
		mat4 r = z * y * x;
		return r;
	}

	mat4 scale(hdk::vec3 scale)
	{
		HDK_PROFZONE("Matrix Scale", 0xFF0000AA);
		mat4 m = mat4::identity();
		for(std::size_t i = 0; i < 3; i++)
		{
			m(i, i) = scale[i];
		}
		return m;
	}

	mat4 model(hdk::vec3 position, hdk::vec3 rotation, hdk::vec3 scale)
	{
		HDK_PROFZONE("Matrix Model", 0xFF0000AA);
		return tz::translate(position) * tz::rotate(rotation) * tz::scale(scale);
	}

	mat4 view(hdk::vec3 position, hdk::vec3 rotation)
	{
		HDK_PROFZONE("Matrix View", 0xFF0000AA);
		return (translate(position) * rotate(rotation)).inverse();
	}

	mat4 perspective(float fov, float aspect_ratio, float near, float far)
	{
		HDK_PROFZONE("Matrix Perspective", 0xFF0000AA);
		const float thf = std::tan(fov / 2.0f);
		mat4 m = mat4::identity();
		m(0, 0) = 1.0f / (aspect_ratio * thf);
		m(1, 1) = 1.0f / thf;

		m(2, 2) = (far + near) / (near - far);
		m(3, 2) = -1.0f;
		
		m(2, 3) = (2.0f * far * near) / (near - far);
		m(3, 3) = 0.0f;
		return m;
	}

	mat4 orthographic(float left, float right, float top, float bottom, float near, float far)
	{
		HDK_PROFZONE("Matrix Orthographic", 0xFF0000AA);
		mat4 m = mat4::identity();
		m(0, 0) = 2.0f / (right - left);
		m(1, 1) = 2.0f / (top - bottom);
		m(2, 2) = -2.0f / (far - near);

		mat4::Row& bottom_row = m[2];
		bottom_row[0] = -(right + left) / (right - left);
		bottom_row[1] = -(top + bottom) / (top - bottom);
		bottom_row[2] = -(far + near) / (far - near);
		return m;
	}
}
