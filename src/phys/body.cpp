#include "phys/body.hpp"

namespace tz::phys
{
	void Body::update(float delta_millis)
	{
		float delta_seconds = delta_millis / 1000.0f;
		#if 0 // Backwards Euler
		this->velocity += this->acceleration * delta_seconds;
		this->transform.position += this->velocity * delta_seconds;
		#endif

		// Verlet	
		float half_delta = delta_seconds * 0.5f;
		this->transform.position + this->velocity * half_delta;
		this->velocity += this->acceleration * delta_seconds;
		this->transform.position + this->velocity * half_delta;
	}

	float Body::get_speed() const
	{
		return this->velocity.length();
	}

	float Body::get_acceleration_magnitude() const
	{
		return this->acceleration.length();
	}
}
