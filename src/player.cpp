#include "player.hpp"

Player::Player(Camera& cam): Entity(), cam(cam)
{
	
}

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
	this->velocity += (acceleration / fps);
	this->getCamera().getPosR() += (velocity / fps);
}