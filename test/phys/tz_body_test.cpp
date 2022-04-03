#include "core/assert.hpp"
#include "phys/body.hpp"

void scalar_gets();

int main()
{
	scalar_gets();
}

void scalar_gets()
{
	constexpr tz::Vec3 zero{0.0f, 0.0f, 0.0f};
	tz::phys::Body b;
	tz_assert(b.get_speed() == 0, "Default body has nonzero speed %.2f m/s", b.get_speed());
	tz_assert(b.get_acceleration_magnitude() == 0, "Default body has nonzero acceleration %.2f m/s/s", b.get_acceleration_magnitude());

	b.velocity = {0.0f, -9.81f, 0.0f};
	tz_assert(b.get_speed() == 9.81f, "Body speed is calculated wrongly. Expected %.2f, got %.2f", 9.81f, b.get_speed());
	tz_assert(b.transform.position == zero, "Default body is not at origin. Expected {%.2f, %.2f, %.2f}, got {%.2f, %.2f, %.2f}", 0.0f, 0.0f, 0.0f, b.transform.position[0], b.transform.position[1], b.transform.position[2]);

	// 1 second passes.
	b.update(1000.0f);

	tz_assert(b.get_speed() == 9.81f, "Body speed changed but didn't have any acceleratsion. Expected %.2f, got %.2f", 9.81f, b.get_speed());
	tz_assert(b.transform.position[1] = -9.81f, "Body did not move correctly when gravity was applied. Expected %.2f, got %.2f", -9.81f, b.transform.position[1]);
}
