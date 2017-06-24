#ifndef PLAYER_HPP
#define PLAYER_HPP
#include "camera.hpp"
#include "entity.hpp"

class Player: public Entity
{
public:
	Player(float mass, Camera& cam);
	Player(const Player& copy) = default;
	Player(Player&& move) = default;
	Player& operator=(const Player& rhs) = default;
	
	void setPosition(Vector3F position);
	const Vector3F& getPosition() const;
	Vector3F& getPositionR();
	Camera& getCamera();
	void updateMotion(unsigned int fps);
private:
	Camera& cam;
};

#endif