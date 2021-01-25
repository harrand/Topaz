#include "geo/boundary/sphere.hpp"
#include "geo/boundary/aabb.hpp"
#include <algorithm>

namespace tz::geo
{
    BoundarySphere::BoundarySphere(tz::Vec3 centre, float radius): centre(centre), radius(radius){}

    bool BoundarySphere::collides(const tz::Vec3& point) const
    {
        return (this->centre - point).length() <= radius;
    }

    bool BoundarySphere::collides(const BoundarySphere& sphere) const
    {
        float dist_between_centres = (sphere.centre - this->centre).length();
        float total_radii = sphere.radius + this->radius;
        return dist_between_centres <= total_radii;
    }

    bool BoundarySphere::collides(const BoundaryAABB& box) const
    {
        // Retrieve a point within the box which is as close as possible to the sphere.
        auto maxmin = [this, &box](std::size_t idx)
        {
            return std::max(box.get_minimum()[idx], std::min(this->centre[idx], box.get_maximum()[idx]));
        };
        tz::Vec3 closest_point{maxmin(0), maxmin(1), maxmin(2)};
        // Then check if this point intersects with the sphere.
        return this->collides(closest_point);
    }
}