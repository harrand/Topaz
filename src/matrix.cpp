#include "matrix.hpp"

// Matrix2x2

Matrix2x2::Matrix2x2(Vector2F x, Vector2F y): x(std::move(x)), y(std::move(y)){}

Vector2F Matrix2x2::getRowX() const
{
	return this->x;
}

Vector2F Matrix2x2::getRowY() const
{
	return this->y;
}

void Matrix2x2::setRowX(Vector2F row_x)
{
	this->x = row_x;
}

void Matrix2x2::setRowY(Vector2F row_y)
{
	this->y = row_y;
}

float Matrix2x2::determinant() const
{
	return (this->x.getX() * this->y.getY()) - (this->x.getY() * this->y.getX());
}

// Matrix3x3

Matrix3x3::Matrix3x3(Vector3F x, Vector3F y, Vector3F z): x(std::move(x)), y(std::move(y)), z(std::move(z)){}

Vector3F Matrix3x3::getRowX() const
{
	return this->x;
}

Vector3F Matrix3x3::getRowY() const
{
	return this->y;
}

Vector3F Matrix3x3::getRowZ() const
{
	return this->z;
}

void Matrix3x3::setRowX(Vector3F row_x)
{
	this->x = row_x;
}

void Matrix3x3::setRowY(Vector3F row_y)
{
	this->y = row_y;
}

void Matrix3x3::setRowZ(Vector3F row_z)
{
	this->z = row_z;
}

float Matrix3x3::determinant() const
{
	return ((this->x).getX() * (((this->y).getY() * (this->z).getZ()) - ((this->y).getZ() * (this->z).getY()))) - ((this->x).getY() * (((this->y).getX() * (this->z).getZ()) - ((this->y).getZ() * (this->z).getX()))) + ((this->x).getZ() * (((this->y).getX() * (this->z).getY()) - ((this->y).getY() * (this->z).getX())));
}

// Matrix4x4

Matrix4x4::Matrix4x4(Vector4F x, Vector4F y, Vector4F z, Vector4F w): x(std::move(x)), y(std::move(y)), z(std::move(z)), w(std::move(w)){}

Matrix4x4 Matrix4x4::identity()
{
	return {Vector4F(1, 0, 0, 0), Vector4F(0, 1, 0, 0), Vector4F(0, 0, 1, 0), Vector4F(0, 0, 0, 1)};
}

Vector4F Matrix4x4::getRowX() const
{
	return this->x;
}

Vector4F Matrix4x4::getRowY() const
{
	return this->y;
}

Vector4F Matrix4x4::getRowZ() const
{
	return this->z;
}

Vector4F Matrix4x4::getRowW() const
{
	return this->w;
}

void Matrix4x4::setRowX(Vector4F row_x)
{
	this->x = row_x;
}

void Matrix4x4::setRowY(Vector4F row_y)
{
	this->y = row_y;
}

void Matrix4x4::setRowZ(Vector4F row_z)
{
	this->z = row_z;
}

void Matrix4x4::setRowW(Vector4F row_w)
{
	this->w = row_w;
}

Matrix4x4 Matrix4x4::transposed() const
{
	// optimiser should omit these copies
	Vector4F tx(this->x.getX(), this->y.getX(), this->z.getX(), this->w.getX());
	Vector4F ty(this->x.getY(), this->y.getY(), this->z.getY(), this->w.getY());
	Vector4F tz(this->x.getZ(), this->y.getZ(), this->z.getZ(), this->w.getZ());
	Vector4F tw(this->x.getW(), this->y.getW(), this->z.getW(), this->w.getW());
	return {tx, ty, tz, tw};
}

std::array<float, 16> Matrix4x4::fillData() const
{
	// super cheap
	return {{this->x.getX(), this->x.getY(), this->x.getZ(), this->x.getW(), this->y.getX(), this->y.getY(), this->y.getZ(), this->y.getW(), this->z.getX(), this->z.getY(), this->z.getZ(), this->z.getW(), this->w.getX(), this->w.getY(), this->w.getZ(), this->w.getW()}};
}

Matrix3x3 Matrix4x4::subMatrix(float posI, float posJ) const
{
	// your guess is as good as mine. can apply kramer's rule on 3x3 matrices so we split a 4x4 matrix into multiple 3x3s to perform kramers rule on each to find the inverse of the 4x4
	Vector3F x(0, 0, 0), y(0, 0, 0), z(0, 0, 0);
	int di, dj, si, sj;
	for(di = 0; di < 3; di++)
	{
		for(dj = 0; dj < 3; dj++)
		{
			si = di + ((di >= posI) ? 1 : 0);
			sj = dj + ((dj >= posJ) ? 1 : 0);
			
			const Vector3F* row3;
			const Vector4F* row4;
			const float* pos3;
			const float* pos4;
			switch(di)
			{
				case 0:
				row3 = &x;
				break;
				case 1:
				row3 = &y;
				break;
				case 2:
				row3 = &z;
				break;
			}
			
			switch(dj)
			{
				case 0:
				pos3 = &(row3->getX());
				break;
				case 1:
				pos3 = &(row3->getY());
				break;
				case 2:
				pos3 = &(row3->getZ());
				break;
			}
			
			switch(si)
			{
				case 0:
				row4 = &(this->x);
				break;
				case 1:
				row4 = &(this->y);
				break;
				case 2:
				row4 = &(this->z);
				break;
				case 3:
				row4 = &(this->w);
				break;
			}
			
			switch(sj)
			{
				case 0:
				pos4 = &(row4->getX());
				break;
				case 1:
				pos4 = &(row4->getY());
				break;
				case 2:
				pos4 = &(row4->getZ());
				break;
				case 3:
				pos4 = &(row4->getW());
				break;
			}
			// Not evil; no UB as Vector4F members are non-const but using a const reference getter.
			*const_cast<float*>(pos3) = *const_cast<float*>(pos4);
		}
	}
	return {x, y, z};
}

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const
{
	return {this->x + other.getRowX(), this->y + other.getRowY(), this->z + other.getRowZ(), this->w + other.getRowW()};
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const
{
	return {this->x - other.getRowX(), this->y - other.getRowY(), this->z - other.getRowZ(), this->w - other.getRowW()};
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
				*current_result += row.getX() * other.getX();
				break;
				case 1:
				*current_result += row.getY() * other.getY();
				break;
				case 2:
				*current_result += row.getZ() * other.getZ();
				break;
				case 3:
				*current_result += row.getW() * other.getW();
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
	return ((this->x).getX() * (((this->y).getY() * (this->z).getZ()) - ((this->y).getZ() * (this->z).getY()))) - ((this->x).getY() * (((this->y).getX() * (this->z).getZ()) - ((this->y).getZ() * (this->z).getX()))) + ((this->x).getZ() * (((this->y).getX() * (this->z).getY()) - ((this->y).getY() * (this->z).getX())));
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
			Matrix3x3 sub = this->subMatrix(i, j);
			
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
				pos = &(row->getX());
				break;
				case 1:
				pos = &(row->getY());
				break;
				case 2:
				pos = &(row->getZ());
				break;
				case 3:
				pos = &(row->getW());
				break;
			}
			// Not evil, check Matrix4x4::subMatrix explanation.
			*const_cast<float*>(pos) = (sub.determinant() * sign) / det;
		}
	}
	return {x, y, z, w};
}

Matrix4x4 Matrix4x4::createTranslationMatrix(Vector3F position)
{
	Matrix4x4 res(Vector4F(1.0f, 0.0f, 0.0f, position.getX()), Vector4F(0.0f, 1.0f, 0.0f, position.getY()), Vector4F(0.0f, 0.0f, 1.0f, position.getZ()), Vector4F(0.0f, 0.0f, 0.0f, 1.0f));
	/*
	res.getRowXR().setW(position.getX());
	res.getRowYR().setW(position.getY());
	res.getRowZR().setW(position.getZ());
	*/
	return res;
}

Matrix4x4 Matrix4x4::createRotationalXMatrix(float eulerX)
{
	return {Vector4F(1, 0, 0, 0), Vector4F(0, cos(eulerX), -sin(eulerX), 0), Vector4F(0, sin(eulerX), cos(eulerX), 0), Vector4F(0, 0, 0, 1)};
}

Matrix4x4 Matrix4x4::createRotationalYMatrix(float eulerY)
{
	return {Vector4F(cos(eulerY), 0, -sin(eulerY), 0), Vector4F(0, 1, 0, 0), Vector4F(sin(eulerY), 0, cos(eulerY), 0), Vector4F(0, 0, 0, 1)};
}

Matrix4x4 Matrix4x4::createRotationalZMatrix(float eulerZ)
{
	return {Vector4F(cos(eulerZ), -sin(eulerZ), 0, 0), Vector4F(sin(eulerZ), cos(eulerZ), 0, 0), Vector4F(0, 0, 1, 0), Vector4F(0, 0, 0, 1)};
}

Matrix4x4 Matrix4x4::createRotationalMatrix(Vector3F euler_rotation)
{
	return Matrix4x4::createRotationalZMatrix(euler_rotation.getZ()) * Matrix4x4::createRotationalYMatrix(euler_rotation.getY()) * Matrix4x4::createRotationalXMatrix(euler_rotation.getX());
}

Matrix4x4 Matrix4x4::createScalingMatrix(Vector3F scale)
{
	return {Vector4F(scale.getX(), 0, 0, 0), Vector4F(0, scale.getY(), 0, 0), Vector4F(0, 0, scale.getZ(), 0), Vector4F(0, 0, 0, 1)};
}

Matrix4x4 Matrix4x4::createModelMatrix(Vector3F position, Vector3F euler_rotation, Vector3F scale)
{
	return Matrix4x4::createTranslationMatrix(position) * Matrix4x4::createRotationalMatrix(euler_rotation) * Matrix4x4::createScalingMatrix(scale);
}

Matrix4x4 Matrix4x4::createViewMatrix(Vector3F camera_position, Vector3F camera_euler_rotation)
{
	return Matrix4x4::createModelMatrix(camera_position, camera_euler_rotation, Vector3F(1, 1, 1)).inverse();
}

Matrix4x4 Matrix4x4::createOrthographicMatrix(float right, float left, float top, float bottom, float near, float far)
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

Matrix4x4 Matrix4x4::createPerspectiveMatrix(float fov, float aspect_ratio, float nearclip, float farclip)
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

Matrix4x4 Matrix4x4::createPerspectiveMatrix(float fov, float width, float height, float nearclip, float farclip)
{
	return Matrix4x4::createPerspectiveMatrix(fov, width/height, nearclip, farclip);
}
