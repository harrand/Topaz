#ifndef TOPAZ_PHYS_COLLIDERS_COMMON_HPP
#define TOPAZ_PHYS_COLLIDERS_COMMON_HPP
#include "phys/body.hpp"
#include <concepts>

namespace tz::phys
{
	struct ColliderBase
	{
		Body body;
	};

	template<typename T>
	concept Collider = requires(T t)
	{
		requires std::derived_from<T, ColliderBase>;
	};


	template<Collider A, Collider B>
	struct CollisionInfo{};

	template<Collider A, Collider B>
	struct Collision
	{
		const A& a;
		const B& b;
	};

	template<Collider A, Collider B>
	CollisionInfo<A, B> collide(Collision<A, B> collision) = delete;
}

#endif // TOPAZ_PHYS_COLLIDERS_COMMON_HPP
