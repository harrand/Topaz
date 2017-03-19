#ifndef PLAYER_HPP
#define PLAYER_HPP
#include "camera.hpp"
#include "entity.hpp"

class Player: public Entity
{
public:
	Player(float mass, Camera& cam);
	void setPosition(Vector3F position);
	Vector3F getPosition() const;
	Camera& getCamera();
	void updateMotion(unsigned int fps);
private:
	Camera& cam;
};

#endif