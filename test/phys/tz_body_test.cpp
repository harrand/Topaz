#include "core/assert.hpp"
#include "phys/body.hpp"

void scalar_gets();

int main()
{
	scalar_gets();
}

void scalar_gets()
{
	constexpr tz::phys::Body b;
	tz_assert(b.get_speed() == 0, "Default body has nonzero speed %.2f m/s", b.get_speed());
	tz_assert(b.get_acceleration_magnitude() == 0, "Default body has nonzero acceleration %.2f m/s/s", b.get_acceleration_magnitude());
}
