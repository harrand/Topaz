#ifndef TOPAZ_PHYS_COLLIDERS_COMMON_HPP
#define TOPAZ_PHYS_COLLIDERS_COMMON_HPP
#include "phys/body.hpp"
#include <concepts>

namespace tz::phys
{
	/**
	 * @ingroup tz_phys_collider
	 * Represents the base collider, holding only information about the body.
	 */
	struct ColliderBase
	{
		/// Motion information.
		Body body;
	};

	template<typename T>
	concept Collider = requires(T t)
	{
		requires std::derived_from<T, ColliderBase>;
	};

	/**
	 * @ingroup tz_phys_collider
	 * Represents information about a collision between two colliders.
	 * 
	 * Each collider combination should exist as its own specialisation. The use of the generic CollisionInfo<A, B> is ill-formed.
	 */
	template<Collider A, Collider B>
	struct CollisionInfo{};

	/**
	 * @ingroup tz_phys_collider
	 * Represents a pair of colliders which may or may not be colliding.
	 *
	 * CollisionScenarios only concern the colliders involved, the actual information about the resultant collision should be returned via @ref tz::phys::collide
	 */
	template<Collider A, Collider B>
	struct CollisionScenario
	{
		const A& a;
		const B& b;
	};

	/**
	 * @ingroup tz_phys_collider
	 * Perform collision detection for an existing collision scenario.
	 *
	 * Each collider combination should provide its own specialisation of this function. The use of the generic collide<A, B> is ill-formed.
	 */
	template<Collider A, Collider B>
	CollisionInfo<A, B> collide(CollisionScenario<A, B> collision) = delete;
}

#endif // TOPAZ_PHYS_COLLIDERS_COMMON_HPP
