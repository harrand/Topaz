#include "player.hpp"

Player::Player(float mass, Camera& cam): Entity(mass), cam(cam){}

void Player::setPosition(Vector3F position)
{
	this->cam.getPosR() = position;
}

const Vector3F& Player::getPosition() const
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