#include "data/matrix.hpp"
// Matrix2x2

Matrix2x2::Matrix2x2(Vector2F x, Vector2F y): x(std::move(x)), y(std::move(y)){}

float Matrix2x2::determinant() const
{
	return (this->x.x * this->y.y) - (this->x.y * this->y.x);
}

// Matrix3x3

Matrix3x3::Matrix3x3(Vector3F x, Vector3F y, Vector3F z): x(std::move(x)), y(std::move(y)), z(std::move(z)){}

float Matrix3x3::determinant() const
{
	return ((this->x).x * (((this->y).y * (this->z).z) - ((this->y).z * (this->z).y))) - ((this->x).y * (((this->y).x * (this->z).z) - ((this->y).z * (this->z).x))) + ((this->x).z * (((this->y).x * (this->z).y) - ((this->y).y * (this->z).x)));
}

// Matrix4x4

Matrix4x4::Matrix4x4(Vector4F x, Vector4F y, Vector4F z, Vector4F w): x(std::move(x)), y(std::move(y)), z(std::move(z)), w(std::move(w)){}

Matrix4x4 Matrix4x4::identity()
{
	return {Vector4F(1, 0, 0, 0), Vector4F(0, 1, 0, 0), Vector4F(0, 0, 1, 0), Vector4F(0, 0, 0, 1)};
}

Matrix4x4 Matrix4x4::transposed() const
{
	// optimiser should omit these copies
	Vector4F tx(this->x.x, this->y.x, this->z.x, this->w.x);
	Vector4F ty(this->x.y, this->y.y, this->z.y, this->w.y);
	Vector4F tz(this->x.z, this->y.z, this->z.z, this->w.z);
	Vector4F tw(this->x.w, this->y.w, this->z.w, this->w.w);
	return {tx, ty, tz, tw};
}

std::array<float, 16> Matrix4x4::fill_data() const
{
	// super cheap
	return {{this->x.x, this->x.y, this->x.z, this->x.w, this->y.x, this->y.y, this->y.z, this->y.w, this->z.x, this->z.y, this->z.z, this->z.w, this->w.x, this->w.y, this->w.z, this->w.w}};
}

Matrix3x3 Matrix4x4::sub_matrix(float pos_i, float pos_j) const
{
	// your guess is as good as mine. can apply kramer's rule on 3x3 matrices so we split a 4x4 matrix into multiple 3x3s to perform kramers rule on each to find the inverse of the 4x4
	Vector3F x(0, 0, 0), y(0, 0, 0), z(0, 0, 0);
	int di, dj, si, sj;
	for(di = 0; di < 3; di++)
	{
		for(dj = 0; dj < 3; dj++)
		{
			si = di + ((di >= pos_i) ? 1 : 0);
			sj = dj + ((dj >= pos_j) ? 1 : 0);
			
			const Vector3F* row_3;
			const Vector4F* row_4;
			const float* pos_3;
			const float* pos_4;
			switch(di)
			{
				case 0:
				row_3 = &x;
				break;
				case 1:
				row_3 = &y;
				break;
				case 2:
				row_3 = &z;
				break;
			}
			
			switch(dj)
			{
				case 0:
				pos_3 = &(row_3->x);
				break;
				case 1:
				pos_3 = &(row_3->y);
				break;
				case 2:
				pos_3 = &(row_3->z);
				break;
			}
			
			switch(si)
			{
				case 0:
				row_4 = &(this->x);
				break;
				case 1:
				row_4 = &(this->y);
				break;
				case 2:
				row_4 = &(this->z);
				break;
				case 3:
				row_4 = &(this->w);
				break;
			}
			
			switch(sj)
			{
				case 0:
				pos_4 = &(row_4->x);
				break;
				case 1:
				pos_4 = &(row_4->y);
				break;
				case 2:
				pos_4 = &(row_4->z);
				break;
				case 3:
				pos_4 = &(row_4->w);
				break;
			}
			// Not evil; no UB as Vector4F members are non-const but using a const reference getter.
			*const_cast<float*>(pos_3) = *const_cast<float*>(pos_4);
		}
	}
	return {x, y, z};
}

Vector4F Matrix4x4::operator*(const Vector4F& other) const
{
	float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
	float* current_result;
	for(unsigned int row_iterator = 0; row_iterator < 4; row_iterator++)
	{
		Vector4F row;
		switch(row_iterator)
		{
			case 0:
			row = this->x;
			current_result = &x;
			break;
			case 1:
			row = this->y;
			current_result = &y;
			break;
			case 2:
			row = this->z;
			current_result = &z;
			break;
			case 3:
			row = this->w;
			current_result = &w;
			break;
		}
		
		for(unsigned int row_element_iterator = 0; row_element_iterator < 4; row_element_iterator++)
		{
			switch(row_element_iterator)
			{
				case 0:
				*current_result += row.x * other.x;
				break;
				case 1:
				*current_result += row.y * other.y;
				break;
				case 2:
				*current_result += row.z * other.z;
				break;
				case 3:
				*current_result += row.w * other.w;
				break;
			}
		}
	}
	return {x, y, z, w};
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
{
	Matrix4x4 oth = other.transposed();
	Vector4F x(0.0f, 0.0f, 0.0f, 0.0f);
	Vector4F y(0.0f, 0.0f, 0.0f, 0.0f);
	Vector4F z(0.0f, 0.0f, 0.0f, 0.0f);
	Vector4F w(0.0f, 0.0f, 0.0f, 0.0f);
	for(unsigned int row_iterator = 0; row_iterator < 4; row_iterator++)
	{
		Vector4F row;
		switch(row_iterator)
		{
			case 0:
			row = this->x;
			x = oth*row;
			break;
			case 1:
			row = this->y;
			y = oth*row;
			break;
			case 2:
			row = this->z;
			z = oth*row;
			break;
			case 3:
			row = this->w;
			w = oth*row;
			break;	
		}
	}
	return {x, y, z, w};
}

float Matrix4x4::determinant() const
{
	return ((this->x).x * (((this->y).y * (this->z).z) - ((this->y).z * (this->z).y))) - ((this->x).y * (((this->y).x * (this->z).z) - ((this->y).z * (this->z).x))) + ((this->x).z * (((this->y).x * (this->z).y) - ((this->y).y * (this->z).x)));
}

Matrix4x4 Matrix4x4::inverse() const
{
	Vector4F x(0, 0, 0, 0), y(0, 0, 0, 0), z(0, 0, 0, 0), w(0, 0, 0, 0);
	float det = this->determinant();
	if(det < 0.0005)
	{
		return Matrix4x4::identity();
	}
	
	for(unsigned int i = 0; i < 4; i++)
	{
		for(unsigned int j = 0; j < 4; j++)
		{
			int sign = 1 - ((i + j) % 2) * 2;
			Matrix3x3 sub = this->sub_matrix(i, j);
			
			const Vector4F* row;
			const float* pos;
			switch(j)
			{
				case 0:
				row = &x;
				break;
				case 1:
				row = &y;
				break;
				case 2:
				row = &z;
				break;
				case 3:
				row = &w;
				break;
			}
			switch(i)
			{
				case 0:
				pos = &(row->x);
				break;
				case 1:
				pos = &(row->y);
				break;
				case 2:
				pos = &(row->z);
				break;
				case 3:
				pos = &(row->w);
				break;
			}
			// Not evil, check Matrix4x4::sub_matrix explanation.
			*const_cast<float*>(pos) = (sub.determinant() * sign) / det;
		}
	}
	return {x, y, z, w};
}

namespace tz::transform
{
	Matrix4x4 translate(const Vector3F& position)
	{
		Matrix4x4 res(Vector4F(1.0f, 0.0f, 0.0f, position.x), Vector4F(0.0f, 1.0f, 0.0f, position.y), Vector4F(0.0f, 0.0f, 1.0f, position.z), Vector4F(0.0f, 0.0f, 0.0f, 1.0f));
		return res;
	}
	
	Matrix4x4 rotate_x(float euler_x)
	{
		return {Vector4F(1, 0, 0, 0), Vector4F(0, cos(euler_x), -sin(euler_x), 0), Vector4F(0, sin(euler_x), cos(euler_x), 0), Vector4F(0, 0, 0, 1)};
	}
	
	Matrix4x4 rotate_y(float euler_y)
	{
		return {Vector4F(cos(euler_y), 0, -sin(euler_y), 0), Vector4F(0, 1, 0, 0), Vector4F(sin(euler_y), 0, cos(euler_y), 0), Vector4F(0, 0, 0, 1)};
	}
	
	Matrix4x4 rotate_z(float euler_z)
	{
		return {Vector4F(cos(euler_z), -sin(euler_z), 0, 0), Vector4F(sin(euler_z), cos(euler_z), 0, 0), Vector4F(0, 0, 1, 0), Vector4F(0, 0, 0, 1)};
	}
	
	Matrix4x4 rotate(const Vector3F& euler_rotation)
	{
		using namespace tz::transform;
		return rotate_z(euler_rotation.z) * rotate_y(euler_rotation.y) * rotate_x(euler_rotation.x);
	}

    Vector3F decompose_rotation(const Matrix4x4 rotational_matrix)
    {
    	float sin_pitch = -rotational_matrix.x.z;
    	float cos_pitch = static_cast<float>(std::sqrt(1 - std::pow(sin_pitch, 2)));
    	float sin_roll, cos_roll, sin_yaw, cos_yaw;
		if (std::abs(cos_pitch) <= std::numeric_limits<float>::epsilon())
		{
			sin_roll = -rotational_matrix.z.y;
			cos_roll = rotational_matrix.y.y;
			sin_yaw = 0.0f;
			cos_yaw = 1.0f;
		}
		else
		{
			sin_roll = rotational_matrix.y.z / cos_pitch;
			cos_roll = rotational_matrix.z.z / cos_pitch;
			sin_yaw = rotational_matrix.x.y / cos_pitch;
			cos_yaw = rotational_matrix.x.x / cos_pitch;
		}
		return {static_cast<float>(std::atan2(sin_pitch, cos_pitch))
			  , static_cast<float>(std::atan2(sin_yaw, cos_yaw))
			  , static_cast<float>(std::atan2(sin_roll, cos_roll))};
    }

    Matrix4x4 scale(const Vector3F& scale)
	{
		return {Vector4F(scale.x, 0, 0, 0), Vector4F(0, scale.y, 0, 0), Vector4F(0, 0, scale.z, 0), Vector4F(0, 0, 0, 1)};
	}

    Matrix4x4 look_at(const Vector3F& position, const Vector3F& target, const Vector3F& up)
    {
		Vector3F f = (target - position).normalised();
		Vector3F u = up.normalised();
		Vector3F s = f.cross(u).normalised();
		u = s.cross(f);

		Matrix4x4 result;
		result.x = {s, 0};
		result.y = {u, 0};
		result.z = {f * -1.0f, 0};
		result.w.x = s.dot(position) * -1.0f;//-dot(s, eye);
		result.w.y = u.dot(position) * -1.0f;//-dot(u, eye);
		result.w.z = f.dot(position);//dot(f, eye);
		return result.transposed();
    }
	
	Matrix4x4 model(const Vector3F& position, const Vector3F& euler_rotation, const Vector3F& scale)
	{
		using namespace tz::transform;
		return translate(position) * rotate(euler_rotation) * tz::transform::scale(scale);
	}
	
	Matrix4x4 view(const Vector3F& camera_position, const Vector3F& camera_euler_rotation)
	{
		using namespace tz::transform;
		return model(camera_position, camera_euler_rotation, Vector3F(1, 1, 1)).inverse();
	}
	
	Matrix4x4 orthographic_projection(float right, float left, float top, float bottom, float near, float far)
	{
		using namespace tz::transform;
		/* just following algorithm in row major:
		
		|2/(r-l)		0		0		-(r+l)/(r-l)|
		|	0	 	 2/(t-b)	0		-(t+b)/(t-b)|
		|	0			0	-2/(f-n)	-(f+n)/(f-n)|
		|	0			0		0				1	|
		
		*/
		Vector4F x(2 / (right - left), 0, 0, -1 * ((right + left)/(right - left))), y(0, 2 / (top - bottom), 0, -1 * ((top + bottom)/(top - bottom))), z(0, 0, -2 / (far - near), -1 * ((far + near)/(far - near))), w(0, 0, 0, 1);
		return {x, y, z, w};
	}
	
	Matrix4x4 perspective_projection(float fov, float width, float height, float nearclip, float farclip)
	{
		float aspect_ratio = width / height;
		using namespace tz::transform;
		/* just following algorithm in row major:
		
		|1/(w/h*tan(fov/2))			0		0				0	 |
		|			0		1/tan(fov/2)	0				0	 |
		|			0				0	(f+n)/(n-f) (2*f*n)/(n-f)|
		|			0				0	  -1.0				0	 |
		
		*/
		float thf = tan(fov / 2);
		Vector4F x(1/(aspect_ratio * thf), 0, 0, 0), y(0, 1/thf, 0, 0), z(0, 0, (farclip + nearclip)/(nearclip - farclip), (2 * farclip * nearclip)/(nearclip - farclip)), w(0, 0, -1.0f, 0);
		return {x, y, z, w};
	}
}