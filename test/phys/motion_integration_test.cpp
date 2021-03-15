//
// Created by Harrand on 15/03/2021.
//

#include "test_framework.hpp"
#include "phys/body.hpp"

TZ_TEST_BEGIN(gravity)
    tz::phys::Body weight_1kg
    {
        tz::Vec3{},
        tz::Vec3{},
        tz::Vec3{},
        1.0f
    };

    tz::phys::World world;
    world.add_body(weight_1kg);
    // Add gravity of 10 m/s^2
    tz::phys::World::UniformForceID grav_id = world.register_uniform_force({0.0f, -10.0f, 0.0f});
    topaz_expectf(weight_1kg.position[1] == 0.0f, "A phys::Body was moved within a world before any updates were performed. Expected y-coord to be %g, got %g", 0.0f, weight_1kg.position[1]);

    // We ran for 10 seconds.
    world.update(10000.0f);
    topaz_expectf(weight_1kg.position[1] < 0.0f, "A phys::Body did not move within a world despite a uniform force applied.. Expected y-coord to be over %g, got %g", 0.0f, weight_1kg.position[1]);
    //          dx       = v0t + 0.5  *   a   *      t^2
    float expected_change_in_y = 0.5f * 10.0f * (10.0f * 10.0f);
    topaz_expectf(weight_1kg.position[1] == -expected_change_in_y, "A phys::Body did not move as expected given a uniform force and known mass. Expected y-coord to be %g or below, but instead it was %g.", -expected_change_in_y, weight_1kg.position[1]);
TZ_TEST_END

int main()
{
	tz::test::Unit motion;
	motion.add(gravity());
	return motion.result();
}