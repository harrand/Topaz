#ifndef TOPAZ_PHYS_MOTION_HPP
#define TOPAZ_PHYS_MOTION_HPP

namespace tz::phys
{
    // Pre-define
    struct Body;

    namespace motion
    {
        void verlet_integrate(float delta_millis, Body& body);
    }
}

#endif // TOPAZ_PHYS_MOTION_HPP