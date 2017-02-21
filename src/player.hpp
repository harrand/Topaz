#ifndef PLAYER_HPP
#define PLAYER_HPP
#include "camera.hpp"
#include "entity.hpp"

class Player: public Entity
{
public:
	Player(std::shared_ptr<World>& world, float mass, Camera& cam);
	void setPosition(Vector3F position);
	Vector3F getPosition();
	Camera& getCamera();
	void updateMotion(unsigned int fps);
private:
	Camera& cam;
};

#endif