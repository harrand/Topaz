#ifndef TOPAZ_GEO_SPHERE_HPP
#define TOPAZ_GEO_SPHERE_HPP
#include "core/vector.hpp"

namespace tz::geo
{
    class BoundaryAABB;
    /**
	 * \addtogroup tz_geo Topaz Geometry Library (tz::geo)
	 * A collection of geometric data structures and mathematical types, such as vectors and matrices.
	 * @{
	 */

    class BoundarySphere
    {
    public:
        BoundarySphere(tz::Vec3 centre, float radius);
        bool collides(const tz::Vec3& point) const;
        bool collides(const BoundarySphere& sphere) const;
        bool collides(const BoundaryAABB& box) const;
    private:
        tz::Vec3 centre;
        float radius;
    };
}

#endif // TOPAZ_GEO_SPHERE_HPP