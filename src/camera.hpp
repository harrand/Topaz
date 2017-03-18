#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "matrix.hpp"

class Camera
{
public:
	Camera(Vector3F pos = Vector3F(), Vector3F rot = Vector3F());
	Vector3F getPos() const;
	Vector3F getRot() const;
	Vector3F& getPosR();
	Vector3F& getRotR();
	
	Vector3F getForward() const;
	Vector3F getBackward() const;
	Vector3F getUp() const;
	Vector3F getDown() const;
	Vector3F getLeft() const;
	Vector3F getRight() const;
private:
	Matrix4x4 getCameraMatrix() const;
	Vector3F pos, rot;
};

#endif