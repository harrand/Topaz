#include "matrix.hpp"

// Matrix2x2

Matrix2x2::Matrix2x2(Vector2F x, Vector2F y): x(std::move(x)), y(std::move(y)){}

Vector2F Matrix2x2::get_row_x() const
{
	return this->x;
}

Vector2F Matrix2x2::get_row_y() const
{
	return this->y;
}

void Matrix2x2::set_row_x(Vector2F row_x)
{
	this->x = row_x;
}

void Matrix2x2::set_row_y(Vector2F row_y)
{
	this->y = row_y;
}

float Matrix2x2::determinant() const
{
	return (this->x.get_x() * this->y.get_y()) - (this->x.get_y() * this->y.get_x());
}

// Matrix3x3

Matrix3x3::Matrix3x3(Vector3F x, Vector3F y, Vector3F z): x(std::move(x)), y(std::move(y)), z(std::move(z)){}

Vector3F Matrix3x3::get_row_x() const
{
	return this->x;
}

Vector3F Matrix3x3::get_row_y() const
{
	return this->y;
}

Vector3F Matrix3x3::get_row_z() const
{
	return this->z;
}

void Matrix3x3::set_row_x(Vector3F row_x)
{
	this->x = row_x;
}

void Matrix3x3::set_row_y(Vector3F row_y)
{
	this->y = row_y;
}

void Matrix3x3::set_row_z(Vector3F row_z)
{
	this->z = row_z;
}

float Matrix3x3::determinant() const
{
	return ((this->x).get_x() * (((this->y).get_y() * (this->z).get_z()) - ((this->y).get_z() * (this->z).get_y()))) - ((this->x).get_y() * (((this->y).get_x() * (this->z).get_z()) - ((this->y).get_z() * (this->z).get_x()))) + ((this->x).get_z() * (((this->y).get_x() * (this->z).get_y()) - ((this->y).get_y() * (this->z).get_x())));
}

// Matrix4x4

Matrix4x4::Matrix4x4(Vector4F x, Vector4F y, Vector4F z, Vector4F w): x(std::move(x)), y(std::move(y)), z(std::move(z)), w(std::move(w)){}

Matrix4x4 Matrix4x4::identity()
{
	return {Vector4F(1, 0, 0, 0), Vector4F(0, 1, 0, 0), Vector4F(0, 0, 1, 0), Vector4F(0, 0, 0, 1)};
}

Vector4F Matrix4x4::get_row_x() const
{
	return this->x;
}

Vector4F Matrix4x4::get_row_y() const
{
	return this->y;
}

Vector4F Matrix4x4::get_row_z() const
{
	return this->z;
}

Vector4F Matrix4x4::get_row_w() const
{
	return this->w;
}

void Matrix4x4::set_row_x(Vector4F row_x)
{
	this->x = row_x;
}

void Matrix4x4::set_row_y(Vector4F row_y)
{
	this->y = row_y;
}

void Matrix4x4::set_row_z(Vector4F row_z)
{
	this->z = row_z;
}

void Matrix4x4::set_row_w(Vector4F row_w)
{
	this->w = row_w;
}

Matrix4x4 Matrix4x4::transposed() const
{
	// optimiser should omit these copies
	Vector4F tx(this->x.get_x(), this->y.get_x(), this->z.get_x(), this->w.get_x());
	Vector4F ty(this->x.get_y(), this->y.get_y(), this->z.get_y(), this->w.get_y());
	Vector4F tz(this->x.get_z(), this->y.get_z(), this->z.get_z(), this->w.get_z());
	Vector4F tw(this->x.get_w(), this->y.get_w(), this->z.get_w(), this->w.get_w());
	return {tx, ty, tz, tw};
}

std::array<float, 16> Matrix4x4::fill_data() const
{
	// super cheap
	return {{this->x.get_x(), this->x.get_y(), this->x.get_z(), this->x.get_w(), this->y.get_x(), this->y.get_y(), this->y.get_z(), this->y.get_w(), this->z.get_x(), this->z.get_y(), this->z.get_z(), this->z.get_w(), this->w.get_x(), this->w.get_y(), this->w.get_z(), this->w.get_w()}};
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
				pos_3 = &(row_3->get_x());
				break;
				case 1:
				pos_3 = &(row_3->get_y());
				break;
				case 2:
				pos_3 = &(row_3->get_z());
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
				pos_4 = &(row_4->get_x());
				break;
				case 1:
				pos_4 = &(row_4->get_y());
				break;
				case 2:
				pos_4 = &(row_4->get_z());
				break;
				case 3:
				pos_4 = &(row_4->get_w());
				break;
			}
			// Not evil; no UB as Vector4F members are non-const but using a const reference getter.
			*const_cast<float*>(pos_3) = *const_cast<float*>(pos_4);
		}
	}
	return {x, y, z};
}

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const
{
	return {this->x + other.get_row_x(), this->y + other.get_row_y(), this->z + other.get_row_z(), this->w + other.get_row_w()};
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const
{
	return {this->x - other.get_row_x(), this->y - other.get_row_y(), this->z - other.get_row_z(), this->w - other.get_row_w()};
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
				*current_result += row.get_x() * other.get_x();
				break;
				case 1:
				*current_result += row.get_y() * other.get_y();
				break;
				case 2:
				*current_result += row.get_z() * other.get_z();
				break;
				case 3:
				*current_result += row.get_w() * other.get_w();
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
	return ((this->x).get_x() * (((this->y).get_y() * (this->z).get_z()) - ((this->y).get_z() * (this->z).get_y()))) - ((this->x).get_y() * (((this->y).get_x() * (this->z).get_z()) - ((this->y).get_z() * (this->z).get_x()))) + ((this->x).get_z() * (((this->y).get_x() * (this->z).get_y()) - ((this->y).get_y() * (this->z).get_x())));
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
				pos = &(row->get_x());
				break;
				case 1:
				pos = &(row->get_y());
				break;
				case 2:
				pos = &(row->get_z());
				break;
				case 3:
				pos = &(row->get_w());
				break;
			}
			// Not evil, check Matrix4x4::sub_matrix explanation.
			*const_cast<float*>(pos) = (sub.determinant() * sign) / det;
		}
	}
	return {x, y, z, w};
}

Matrix4x4 Matrix4x4::create_translation_matrix(Vector3F position)
{
	Matrix4x4 res(Vector4F(1.0f, 0.0f, 0.0f, position.get_x()), Vector4F(0.0f, 1.0f, 0.0f, position.get_y()), Vector4F(0.0f, 0.0f, 1.0f, position.get_z()), Vector4F(0.0f, 0.0f, 0.0f, 1.0f));
	return res;
}

Matrix4x4 Matrix4x4::create_rotational_x_matrix(float euler_x)
{
	return {Vector4F(1, 0, 0, 0), Vector4F(0, cos(euler_x), -sin(euler_x), 0), Vector4F(0, sin(euler_x), cos(euler_x), 0), Vector4F(0, 0, 0, 1)};
}

Matrix4x4 Matrix4x4::create_rotational_y_matrix(float euler_y)
{
	return {Vector4F(cos(euler_y), 0, -sin(euler_y), 0), Vector4F(0, 1, 0, 0), Vector4F(sin(euler_y), 0, cos(euler_y), 0), Vector4F(0, 0, 0, 1)};
}

Matrix4x4 Matrix4x4::create_rotational_z_matrix(float euler_z)
{
	return {Vector4F(cos(euler_z), -sin(euler_z), 0, 0), Vector4F(sin(euler_z), cos(euler_z), 0, 0), Vector4F(0, 0, 1, 0), Vector4F(0, 0, 0, 1)};
}

Matrix4x4 Matrix4x4::create_rotational_matrix(Vector3F euler_rotation)
{
	return Matrix4x4::create_rotational_z_matrix(euler_rotation.get_z()) * Matrix4x4::create_rotational_y_matrix(euler_rotation.get_y()) * Matrix4x4::create_rotational_x_matrix(euler_rotation.get_x());
}

Matrix4x4 Matrix4x4::create_scaling_matrix(Vector3F scale)
{
	return {Vector4F(scale.get_x(), 0, 0, 0), Vector4F(0, scale.get_y(), 0, 0), Vector4F(0, 0, scale.get_z(), 0), Vector4F(0, 0, 0, 1)};
}

Matrix4x4 Matrix4x4::create_model_matrix(Vector3F position, Vector3F euler_rotation, Vector3F scale)
{
	return Matrix4x4::create_translation_matrix(position) * Matrix4x4::create_rotational_matrix(euler_rotation) * Matrix4x4::create_scaling_matrix(scale);
}

Matrix4x4 Matrix4x4::create_view_matrix(Vector3F camera_position, Vector3F camera_euler_rotation)
{
	return Matrix4x4::create_model_matrix(camera_position, camera_euler_rotation, Vector3F(1, 1, 1)).inverse();
}

Matrix4x4 Matrix4x4::create_orthographic_matrix(float right, float left, float top, float bottom, float near, float far)
{
	/* just following algorithm in row major:
	
	|2/(r-l)		0		0		-(r+l)/(r-l)|
	|	0	 	 2/(t-b)	0		-(t+b)/(t-b)|
	|	0			0	-2/(f-n)	-(f+n)/(f-n)|
	|	0			0		0				1	|
	
	*/
	Vector4F x(2 / (right - left), 0, 0, -1 * ((right + left)/(right - left))), y(0, 2 / (top - bottom), 0, -1 * ((top + bottom)/(top - bottom))), z(0, 0, -2 / (far - near), -1 * ((far + near)/(far - near))), w(0, 0, 0, 1);
	return {x, y, z, w};
}

Matrix4x4 Matrix4x4::create_perspective_matrix(float fov, float aspect_ratio, float nearclip, float farclip)
{
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

Matrix4x4 Matrix4x4::create_perspective_matrix(float fov, float width, float height, float nearclip, float farclip)
{
	return Matrix4x4::create_perspective_matrix(fov, width/height, nearclip, farclip);
}
