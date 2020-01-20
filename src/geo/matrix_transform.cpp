#include "geo/matrix_transform.hpp"
#include <cmath>

namespace tz::geo
{
    Mat4 translate(Vec3 position)
    {
        Mat4 m = Mat4::identity();
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
    Mat4 rotate_x(float angle)
    {
        Mat4 m = Mat4::identity();
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

    Mat4 rotate_y(float angle)
    {
        Mat4 m = Mat4::identity();
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
    Mat4 rotate_z(float angle)
    {
        Mat4 m = Mat4::identity();
        m(0, 0) = std::cos(angle);
        m(0, 1) = -std::sin(angle);
        m(1, 0) = std::sin(angle);
        m(1, 1) = std::cos(angle);
        return m;
    }

    Mat4 rotate(Vec3 rotation)
    {
        Mat4 x = rotate_x(rotation[0]);
        Mat4 y = rotate_y(rotation[1]);
        Mat4 z = rotate_z(rotation[2]);
        Mat4 r = z * y * x;
        return r;
    }

    Mat4 scale(Vec3 scale)
    {
        Mat4 m = Mat4::identity();
        for(std::size_t i = 0; i < 3; i++)
        {
            m(i, i) = scale[i];
        }
        return m;
    }

    Mat4 model(Vec3 position, Vec3 rotation, Vec3 scale)
    {
        return translate(position) * rotate(rotation) * tz::geo::scale(scale);
    }

    Mat4 view(Vec3 position, Vec3 rotation)
    {
        return (translate(position) * rotate(rotation)).inverse();
    }

    Mat4 perspective(float fov, float aspect_ratio, float near, float far)
    {
        const float thf = std::tan(fov / 2.0f);
        Mat4 m = Mat4::identity();
        m(0, 0) = 1.0f / (aspect_ratio * thf);
        m(1, 1) = 1.0f / thf;

        m(2, 2) = (far + near) / (near - far);
        m(3, 2) = -1.0f;
        
        m(2, 3) = (2.0f * far * near) / (near - far);
        m(3, 3) = 0.0f;
        return m;
    }

    Mat4 orthographic(float left, float right, float top, float bottom, float near, float far)
    {
        Mat4 m = Mat4::identity();
        m(0, 0) = 2.0f / (right - left);
        m(1, 1) = 2.0f / (top - bottom);
        m(2, 2) = -2.0f / (far - near);

        Mat4::Row& bottom_row = m[2];
        bottom_row[0] = -(right + left) / (right - left);
        bottom_row[1] = -(top + bottom) / (top - bottom);
        bottom_row[2] = -(far + near) / (far - near);
        return m;
    }
}