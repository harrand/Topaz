#include "matrix.hpp"

// Matrix2x2

Matrix2x2::Matrix2x2(Vector2F x, Vector2F y)
{
	this->x = x;
	this->y = y;
}

Vector2F Matrix2x2::getRowX()
{
	return this->x;
}

Vector2F Matrix2x2::getRowY()
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

float Matrix2x2::determinant()
{
	return (this->x.getX() * this->y.getY()) - (this->x.getY() * this->y.getX());
}

// Matrix3x3

Matrix3x3::Matrix3x3(Vector3F x, Vector3F y, Vector3F z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3F Matrix3x3::getRowX()
{
	return this->x;
}

Vector3F Matrix3x3::getRowY()
{
	return this->y;
}

Vector3F Matrix3x3::getRowZ()
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

float Matrix3x3::determinant()
{
	return ((this->x).getX() * (((this->y).getY() * (this->z).getZ()) - ((this->y).getZ() * (this->z).getY()))) - ((this->x).getY() * (((this->y).getX() * (this->z).getZ()) - ((this->y).getZ() * (this->z).getX()))) + ((this->x).getZ() * (((this->y).getX() * (this->z).getY()) - ((this->y).getY() * (this->z).getX())));
}

// Matrix4x4

Matrix4x4::Matrix4x4(Vector4F x, Vector4F y, Vector4F z, Vector4F w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Matrix4x4 Matrix4x4::identity()
{
	return Matrix4x4();
}

Vector4F Matrix4x4::getRowX()
{
	return this->x;
}

Vector4F Matrix4x4::getRowY()
{
	return this->y;
}

Vector4F Matrix4x4::getRowZ()
{
	return this->z;
}

Vector4F Matrix4x4::getRowW()
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

Matrix4x4 Matrix4x4::transposed()
{
	Vector4F tx(this->x.getX(), this->y.getX(), this->z.getX(), this->w.getX());
	Vector4F ty(this->x.getY(), this->y.getY(), this->z.getY(), this->w.getY());
	Vector4F tz(this->x.getZ(), this->y.getZ(), this->z.getZ(), this->w.getZ());
	Vector4F tw(this->x.getW(), this->y.getW(), this->z.getW(), this->w.getW());
	return Matrix4x4(tx, ty, tz, tw);
}

Matrix4x4 Matrix4x4::washed(float min, float max)
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

void Matrix4x4::fillData(float data[16])
{
	data[0] = this->x.getX();
	data[1] = this->x.getY();
	data[2] = this->x.getZ();
	data[3] = this->x.getW();
	
	data[4] = this->y.getX();
	data[5] = this->y.getY();
	data[6] = this->y.getZ();
	data[7] = this->y.getW();
	
	data[8] = this->z.getX();
	data[9] = this->z.getY();
	data[10] = this->z.getZ();
	data[11] = this->z.getW();
	
	data[12] = this->w.getX();
	data[13] = this->w.getY();
	data[14] = this->w.getZ();
	data[15] = this->w.getW();
}

Matrix3x3 Matrix4x4::subMatrix(float posI, float posJ)
{
	Vector3F x(0, 0, 0), y(0, 0, 0), z(0, 0, 0);
	int di, dj, si, sj;
	for(di = 0; di < 3; di++)
	{
		for(dj = 0; dj < 3; dj++)
		{
			si = di + ((di >= posI) ? 1 : 0);
			sj = dj + ((dj >= posJ) ? 1 : 0);
			
			Vector3F* row3;
			Vector4F* row4;
			float* pos3;
			float* pos4;
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
				pos3 = &(row3->getXR());
				break;
				case 1:
				pos3 = &(row3->getYR());
				break;
				case 2:
				pos3 = &(row3->getZR());
				break;
			}
			
			switch(si)
			{
				case 0:
				row4 = &(this->getRowXR());
				break;
				case 1:
				row4 = &(this->getRowYR());
				break;
				case 2:
				row4 = &(this->getRowZR());
				break;
				case 3:
				row4 = &(this->getRowWR());
				break;
			}
			
			switch(sj)
			{
				case 0:
				pos4 = &(row4->getXR());
				break;
				case 1:
				pos4 = &(row4->getYR());
				break;
				case 2:
				pos4 = &(row4->getZR());
				break;
				case 3:
				pos4 = &(row4->getWR());
				break;
			}
			*pos3 = *pos4;
		}
	}
	return Matrix3x3(x, y, z);
}

Matrix4x4 Matrix4x4::operator+(Matrix4x4 other)
{
	return Matrix4x4(this->x + other.getRowX(), this->y + other.getRowY(), this->z + other.getRowZ(), this->w + other.getRowW());
}

Matrix4x4 Matrix4x4::operator-(Matrix4x4 other)
{
	return Matrix4x4(this->x - other.getRowX(), this->y - other.getRowY(), this->z - other.getRowZ(), this->w - other.getRowW());
}

Vector4F Matrix4x4::operator*(Vector4F other)
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
			float res = 0.0f;
			switch(rowCounter)
			{
				case 0:
				*curRes += row.getXR() * other.getXR();
				break;
				case 1:
				*curRes += row.getYR() * other.getYR();
				break;
				case 2:
				*curRes += row.getZR() * other.getZR();
				break;
				case 3:
				*curRes += row.getWR() * other.getWR();
				break;
			}
		}
	}
	return Vector4F(x, y, z, w);
}

Matrix4x4 Matrix4x4::operator*(Matrix4x4 other)
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

float Matrix4x4::determinant()
{
	return ((this->x).getX() * (((this->y).getY() * (this->z).getZ()) - ((this->y).getZ() * (this->z).getY()))) - ((this->x).getY() * (((this->y).getX() * (this->z).getZ()) - ((this->y).getZ() * (this->z).getX()))) + ((this->x).getZ() * (((this->y).getX() * (this->z).getY()) - ((this->y).getY() * (this->z).getX())));
}

Matrix4x4 Matrix4x4::inverse()
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
			
			Vector4F* row;
			float* pos;
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
				pos = &(row->getXR());
				break;
				case 1:
				pos = &(row->getYR());
				break;
				case 2:
				pos = &(row->getZR());
				break;
				case 3:
				pos = &(row->getWR());
				break;
			}
			*pos = (sub.determinant() * sign) / det;
		}
	}
	return Matrix4x4(x, y, z, w);
}

// MatrixTransformations

Matrix4x4 MatrixTransformations::createTranslationMatrix(Vector3F position)
{
	Matrix4x4 res = Matrix4x4::identity();
	res.getRowXR().getWR() = position.getX();
	res.getRowYR().getWR() = position.getY();
	res.getRowZR().getWR() = position.getZ();
	return res;
}

Matrix4x4 MatrixTransformations::createRotationalXMatrix(float eulerX)
{
	return Matrix4x4(Vector4F(1, 0, 0, 0), Vector4F(0, cos(eulerX), -sin(eulerX), 0), Vector4F(0, sin(eulerX), cos(eulerX), 0), Vector4F(0, 0, 0, 1));
}

Matrix4x4 MatrixTransformations::createRotationalYMatrix(float eulerY)
{
	return Matrix4x4(Vector4F(cos(eulerY), 0, -sin(eulerY), 0), Vector4F(0, 1, 0, 0), Vector4F(sin(eulerY), 0, cos(eulerY), 0), Vector4F(0, 0, 0, 1));
}

Matrix4x4 MatrixTransformations::createRotationalZMatrix(float eulerZ)
{
	return Matrix4x4(Vector4F(cos(eulerZ), -sin(eulerZ), 0, 0), Vector4F(sin(eulerZ), cos(eulerZ), 0, 0), Vector4F(0, 0, 1, 0), Vector4F(0, 0, 0, 1));
}

Matrix4x4 MatrixTransformations::createRotationalMatrix(Vector3F eulerRotation)
{
	return MatrixTransformations::createRotationalZMatrix(eulerRotation.getZ()) * MatrixTransformations::createRotationalYMatrix(eulerRotation.getY()) * MatrixTransformations::createRotationalXMatrix(eulerRotation.getX());
}

Matrix4x4 MatrixTransformations::createScalingMatrix(Vector3F scale)
{
	return Matrix4x4(Vector4F(scale.getX(), 0, 0, 0), Vector4F(0, scale.getY(), 0, 0), Vector4F(0, 0, scale.getZ(), 0), Vector4F(0, 0, 0, 1));
}

Matrix4x4 MatrixTransformations::createModelMatrix(Vector3F position, Vector3F eulerRotation, Vector3F scale)
{
	return MatrixTransformations::createTranslationMatrix(position) * MatrixTransformations::createRotationalMatrix(eulerRotation) * MatrixTransformations::createScalingMatrix(scale);
}

Matrix4x4 MatrixTransformations::createViewMatrix(Vector3F cameraPosition, Vector3F cameraEulerRotation)
{
	return MatrixTransformations::createModelMatrix(cameraPosition, cameraEulerRotation, Vector3F(1, 1, 1)).inverse();
}

Matrix4x4 MatrixTransformations::createProjectionMatrix(float fov, float aspectRatio, float nearclip, float farclip)
{
	Vector4F x(0, 0, 0, 0), y(0, 0, 0, 0), z(0, 0, 0, 0), w(0, 0, 0, 0);
	float thf = tan(fov / 2);
	x.getXR() = 1/(aspectRatio * thf);
	y.getYR() = 1/thf;
	z.getZR() = (farclip + nearclip)/(nearclip - farclip);
	z.getWR() = (2 * farclip * nearclip)/(nearclip - farclip);
	w.getZR() = -1.0f;
	return Matrix4x4(x, y, z, w);//.transposed();
}

Matrix4x4 MatrixTransformations::createProjectionMatrix(float fov, float width, float height, float nearclip, float farclip)
{
	return MatrixTransformations::createProjectionMatrix(fov, (float)((width)/(height)), nearclip, farclip);
}
