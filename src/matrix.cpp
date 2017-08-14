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

Vector2F& Matrix2x2::getRowXR()
{
	return this->x;
}

Vector2F& Matrix2x2::getRowYR()
{
	return this->y;
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

Vector3F& Matrix3x3::getRowXR()
{
	return this->x;
}

Vector3F& Matrix3x3::getRowYR()
{
	return this->y;
}

Vector3F& Matrix3x3::getRowZR()
{
	return this->z;
}

float Matrix3x3::determinant() const
{
	return ((this->x).getX() * (((this->y).getY() * (this->z).getZ()) - ((this->y).getZ() * (this->z).getY()))) - ((this->x).getY() * (((this->y).getX() * (this->z).getZ()) - ((this->y).getZ() * (this->z).getX()))) + ((this->x).getZ() * (((this->y).getX() * (this->z).getY()) - ((this->y).getY() * (this->z).getX())));
}

// Matrix4x4

Matrix4x4::Matrix4x4(Vector4F x, Vector4F y, Vector4F z, Vector4F w): x(std::move(x)), y(std::move(y)), z(std::move(z)), w(std::move(w)){}

Matrix4x4 Matrix4x4::identity()
{
	return Matrix4x4(Vector4F(1, 0, 0, 0), Vector4F(0, 1, 0, 0), Vector4F(0, 0, 1, 0), Vector4F(0, 0, 0, 1));
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

Vector4F& Matrix4x4::getRowXR()
{
	return this->x;
}

Vector4F& Matrix4x4::getRowYR()
{
	return this->y;
}

Vector4F& Matrix4x4::getRowZR()
{
	return this->z;
}

Vector4F& Matrix4x4::getRowWR()
{
	return this->w;
}

Matrix4x4 Matrix4x4::transposed() const
{
	Vector4F tx(this->x.getX(), this->y.getX(), this->z.getX(), this->w.getX());
	Vector4F ty(this->x.getY(), this->y.getY(), this->z.getY(), this->w.getY());
	Vector4F tz(this->x.getZ(), this->y.getZ(), this->z.getZ(), this->w.getZ());
	Vector4F tw(this->x.getW(), this->y.getW(), this->z.getW(), this->w.getW());
	return Matrix4x4(tx, ty, tz, tw);
}

Matrix4x4 Matrix4x4::washed(float min, float max) const
{
	Matrix4x4 copy(Vector4F(this->x.getX(), this->x.getY(), this->x.getZ(), this->x.getW()), Vector4F(this->y.getX(), this->y.getY(), this->y.getZ(), this->y.getW()), Vector4F(this->z.getX(), this->z.getY(), this->z.getZ(), this->z.getW()), Vector4F(this->w.getX(), this->w.getY(), this->w.getZ(), this->w.getW()));
	Vector4F* row;
	float* val;
	for(unsigned int i = 0; i < 4; i++)
	{
		switch(i)
		{
			case 0:
			row = &(copy.getRowXR());
			break;
			case 1:
			row = &(copy.getRowYR());
			break;
			case 2:
			row = &(copy.getRowZR());
			break;
			case 3:
			row = &(copy.getRowWR());
			break;
		}
		
		for(unsigned int j = 0; j < 4; j++)
		{
			switch(j)
			{
				case 0:
				val = &(row->getXR());
				break;
				case 1:
				val = &(row->getYR());
				break;
				case 2:
				val = &(row->getZR());
				break;
				case 3:
				val = &(row->getWR());
				break;
			}
			if(fabs(*val) < min || fabs(*val) > max)
				*val = 0.0f;
		}
	}
	return copy;
}

std::array<float, 16> Matrix4x4::fillData() const
{
	return std::array<float, 16>({this->x.getX(), this->x.getY(), this->x.getZ(), this->x.getW(), this->y.getX(), this->y.getY(), this->y.getZ(), this->y.getW(), this->z.getX(), this->z.getY(), this->z.getZ(), this->z.getW(), this->w.getX(), this->w.getY(), this->w.getZ(), this->w.getW()});
}

Matrix3x3 Matrix4x4::subMatrix(float posI, float posJ) const
{
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
	return Matrix3x3(x, y, z);
}

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const
{
	return Matrix4x4(this->x + other.getRowX(), this->y + other.getRowY(), this->z + other.getRowZ(), this->w + other.getRowW());
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const
{
	return Matrix4x4(this->x - other.getRowX(), this->y - other.getRowY(), this->z - other.getRowZ(), this->w - other.getRowW());
}

Vector4F Matrix4x4::operator*(const Vector4F& other) const
{
	float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
	float* curRes;
	for(unsigned int rowIter = 0; rowIter < 4; rowIter++)
	{
		Vector4F row;
		switch(rowIter)
		{
			case 0:
			row = this->x;
			curRes = &x;
			break;
			case 1:
			row = this->y;
			curRes = &y;
			break;
			case 2:
			row = this->z;
			curRes = &z;
			break;
			case 3:
			row = this->w;
			curRes = &w;
			break;
		}
		
		for(unsigned int rowCounter = 0; rowCounter < 4; rowCounter++)
		{
			switch(rowCounter)
			{
				case 0:
				*curRes += row.getX() * other.getX();
				break;
				case 1:
				*curRes += row.getY() * other.getY();
				break;
				case 2:
				*curRes += row.getZ() * other.getZ();
				break;
				case 3:
				*curRes += row.getW() * other.getW();
				break;
			}
		}
	}
	return Vector4F(x, y, z, w);
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
{
	Matrix4x4 oth = other.transposed();
	Vector4F x(0.0f, 0.0f, 0.0f, 0.0f);
	Vector4F y(0.0f, 0.0f, 0.0f, 0.0f);
	Vector4F z(0.0f, 0.0f, 0.0f, 0.0f);
	Vector4F w(0.0f, 0.0f, 0.0f, 0.0f);
	for(unsigned int rowIter = 0; rowIter < 4; rowIter++)
	{
		Vector4F row;
		switch(rowIter)
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
	return Matrix4x4(x, y, z, w);
	
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
	return Matrix4x4(x, y, z, w);
}

Matrix4x4 Matrix4x4::createTranslationMatrix(Vector3F position)
{
	Matrix4x4 res = Matrix4x4::identity();
	res.getRowXR().getWR() = position.getX();
	res.getRowYR().getWR() = position.getY();
	res.getRowZR().getWR() = position.getZ();
	return res;
}

Matrix4x4 Matrix4x4::createRotationalXMatrix(float eulerX)
{
	return Matrix4x4(Vector4F(1, 0, 0, 0), Vector4F(0, cos(eulerX), -sin(eulerX), 0), Vector4F(0, sin(eulerX), cos(eulerX), 0), Vector4F(0, 0, 0, 1));
}

Matrix4x4 Matrix4x4::createRotationalYMatrix(float eulerY)
{
	return Matrix4x4(Vector4F(cos(eulerY), 0, -sin(eulerY), 0), Vector4F(0, 1, 0, 0), Vector4F(sin(eulerY), 0, cos(eulerY), 0), Vector4F(0, 0, 0, 1));
}

Matrix4x4 Matrix4x4::createRotationalZMatrix(float eulerZ)
{
	return Matrix4x4(Vector4F(cos(eulerZ), -sin(eulerZ), 0, 0), Vector4F(sin(eulerZ), cos(eulerZ), 0, 0), Vector4F(0, 0, 1, 0), Vector4F(0, 0, 0, 1));
}

Matrix4x4 Matrix4x4::createRotationalMatrix(Vector3F eulerRotation)
{
	return Matrix4x4::createRotationalZMatrix(eulerRotation.getZ()) * Matrix4x4::createRotationalYMatrix(eulerRotation.getY()) * Matrix4x4::createRotationalXMatrix(eulerRotation.getX());
}

Matrix4x4 Matrix4x4::createScalingMatrix(Vector3F scale)
{
	return Matrix4x4(Vector4F(scale.getX(), 0, 0, 0), Vector4F(0, scale.getY(), 0, 0), Vector4F(0, 0, scale.getZ(), 0), Vector4F(0, 0, 0, 1));
}

Matrix4x4 Matrix4x4::createModelMatrix(Vector3F position, Vector3F eulerRotation, Vector3F scale)
{
	return Matrix4x4::createTranslationMatrix(position) * Matrix4x4::createRotationalMatrix(eulerRotation) * Matrix4x4::createScalingMatrix(scale);
}

Matrix4x4 Matrix4x4::createViewMatrix(Vector3F cameraPosition, Vector3F cameraEulerRotation)
{
	return Matrix4x4::createModelMatrix(cameraPosition, cameraEulerRotation, Vector3F(1, 1, 1)).inverse();
}

Matrix4x4 Matrix4x4::createProjectionMatrix(float fov, float aspectRatio, float nearclip, float farclip)
{
	Vector4F x(0, 0, 0, 0), y(0, 0, 0, 0), z(0, 0, 0, 0), w(0, 0, 0, 0);
	float thf = tan(fov / 2);
	x.getXR() = 1/(aspectRatio * thf);
	y.getYR() = 1/thf;
	z.getZR() = (farclip + nearclip)/(nearclip - farclip);
	z.getWR() = (2 * farclip * nearclip)/(nearclip - farclip);
	w.getZR() = -1.0f;
	return Matrix4x4(x, y, z, w);
}

Matrix4x4 Matrix4x4::createProjectionMatrix(float fov, float width, float height, float nearclip, float farclip)
{
	return Matrix4x4::createProjectionMatrix(fov, (float)((width)/(height)), nearclip, farclip);
}
