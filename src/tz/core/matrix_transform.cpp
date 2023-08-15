#include "tz/core/matrix_transform.hpp"
#include "tz/core/profile.hpp"
#include <cmath>

namespace tz
{
	mat4 translate(tz::vec3 position)
	{
		TZ_PROFZONE("Matrix Translate", 0xFF0000AA);
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

	mat4 rotate(tz::vec3 rotation)
	{
		TZ_PROFZONE("Matrix Rotate", 0xFF0000AA);
		mat4 x = rotate_x(rotation[0]);
		mat4 y = rotate_y(rotation[1]);
		mat4 z = rotate_z(rotation[2]);
		mat4 r = z * y * x;
		return r;
	}

	mat4 scale(tz::vec3 scale)
	{
		TZ_PROFZONE("Matrix Scale", 0xFF0000AA);
		mat4 m = mat4::identity();
		for(std::size_t i = 0; i < 3; i++)
		{
			m(i, i) = scale[i];
		}
		return m;
	}

	mat4 model(tz::vec3 position, tz::vec3 rotation, tz::vec3 scale)
	{
		TZ_PROFZONE("Matrix Model", 0xFF0000AA);
		return tz::translate(position) * tz::rotate(rotation) * tz::scale(scale);
	}

	mat4 view(tz::vec3 position, tz::vec3 rotation)
	{
		TZ_PROFZONE("Matrix View", 0xFF0000AA);
		return (translate(position) * rotate(rotation)).inverse();
	}

	mat4 perspective(float fov, float aspect_ratio, float nearval, float farval)
	{
		TZ_PROFZONE("Matrix Perspective", 0xFF0000AA);
		const float thf = std::tan(fov / 2.0f);
		mat4 m = mat4::identity();
		m(0, 0) = 1.0f / (aspect_ratio * thf);
		m(1, 1) = 1.0f / thf;

		m(2, 2) = (farval + nearval) / (nearval - farval);
		m(3, 2) = -1.0f;
		
		m(2, 3) = (2.0f * farval * nearval) / (nearval - farval);
		m(3, 3) = 0.0f;
		return m;
	}

	mat4 orthographic(float left, float right, float top, float bottom, float nearval, float farval)
	{
		TZ_PROFZONE("Matrix Orthographic", 0xFF0000AA);
		mat4 m = mat4::identity();
		m(0, 0) = 2.0f / (right - left);
		m(1, 1) = 2.0f / (top - bottom);
		m(2, 2) = -2.0f / (farval - nearval);

		mat4::Row& bottom_row = m[2];
		bottom_row[0] = -(right + left) / (right - left);
		bottom_row[1] = -(top + bottom) / (top - bottom);
		bottom_row[2] = -(farval + nearval) / (farval - nearval);
		return m;
	}

	tz::vec3 get_rotation_from(const tz::mat4& mat)
	{
		// http://www.opengl-tutorial.org/assets/faq_quaternions/index.html#Q37
		float y = std::asin(mat(2, 0));
		float c = std::cos(y);
		float x, z;
		float ty, tx;
		// y *= RADIANS?
		if(std::abs(c) > 0.005f)
		{
			tx = mat(2, 2) / c;
			ty = -mat(2, 1) / c;
			x = std::atan2(ty, tx); // * RADIANS?
			tx = mat(0, 0) / c;
			ty = -mat(1, 0) / c;
			z = std::atan2(ty, tx); // * RADIANS?
		}
		else
		{
			x = 0.0f;
			tx = mat(1, 1);
			ty = mat(0, 1);
			z = std::atan2(ty, tx);
		}
		if(x < 0.0f) x += (3.14159f * 2);
		if(y < 0.0f) y += (3.14159f * 2);
		if(z < 0.0f) z += (3.14159f * 2);
		return {x, y, z};
	}

	void dbgui_model(tz::mat4& mat)
	{
		mat.dbgui();
		if(ImGui::CollapsingHeader("Model"))
		{
			tz::vec3 pos{mat(0, 3), mat(1, 3), mat(2, 3)};
			tz::vec3 rot = get_rotation_from(mat) * -1.0f;
			tz::vec3 scale{mat(0, 0), mat(1, 1), mat(2, 2)};
			bool b = false;
			b |= ImGui::SliderFloat3("Position", pos.data().data(), -10.0f, 10.0f);
			ImGui::TextColored(ImVec4{1.0f, 1.0f, 0.0f, 1.0f}, "Warning:");
			ImGui::SameLine();
			ImGui::Text("Editing Rotation or Scale is highly likely to result in distortion.");
			b |= ImGui::SliderFloat3("Rotation", rot.data().data(), -3.1f, 3.1f);
			ImGui::SameLine();
			if(ImGui::Button("Reset"))
			{
				mat = tz::model(pos, tz::vec3::zero(), scale);
				return;
			}
			b |= ImGui::SliderFloat3("Scale", scale.data().data(), -5.0f, 5.0f);
			if(b)
			{
				mat = tz::model(pos, rot, scale);
			}
		}
	}

	void dbgui_view(tz::mat4& mat)
	{
		mat.dbgui();
		if(ImGui::CollapsingHeader("View"))
		{
			auto invmat = mat.inverse();
			tz::vec3 pos{invmat(0, 3), invmat(1, 3), invmat(2, 3)};
			tz::vec3 rot = get_rotation_from(invmat) * -1.0f;
			bool b = false;
			static float position_limit_scale = 10.0f;
			ImGui::InputFloat("Position Slider Scale", &position_limit_scale, 0.1f, 0.5f);
			b |= ImGui::SliderFloat3("Position", pos.data().data(), -position_limit_scale, position_limit_scale);
			b |= ImGui::SliderFloat3("Rotation", rot.data().data(), -3.1f, 3.1f);
			ImGui::SameLine();
			if(ImGui::Button("Reset"))
			{
				mat = tz::view(pos, tz::vec3::zero());
				return;
			}
			if(b)
			{
				mat = tz::view(pos, rot);
			}
		}
	}

	void dbgui_perspective(tz::mat4& mat)
	{
		ImGui::Text("Projection widget NYI");
	}
}