#include "player.hpp"

Player::Player(std::shared_ptr<World>& world, float mass, Camera& cam): Entity(world, mass), cam(cam){}

void Player::setPosition(Vector3F position)
{
	this->cam.getPosR() = position;
}

Vector3F Player::getPosition()
{
	return this->cam.getPos();
}

Camera& Player::getCamera()
{
	return this->cam;
}

void Player::updateMotion(unsigned int fps)
{
	this->velocity += (this->getAcceleration() / fps);
	this->getCamera().getPosR() += (velocity / fps);
}