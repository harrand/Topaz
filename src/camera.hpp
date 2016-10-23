#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "matrix.hpp"

class Camera
{
public:
	Camera(Vector3F pos = Vector3F(), Vector3F rot = Vector3F());
	Vector3F getPos();
	Vector3F getRot();
	Vector3F& getPosR();
	Vector3F& getRotR();
	
	Vector3F getForward();
	Vector3F getBackward();
	Vector3F getUp();
	Vector3F getDown();
	Vector3F getLeft();
	Vector3F getRight();
private:
	//Vector4F getPosHomo();
	//Vector4F getRotHomo();
	Matrix4x4 getCameraMatrix();
	Vector3F pos, rot;
};

#endif