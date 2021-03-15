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

    /**
     * Represents a sphere in 3D space.
     */
    class BoundarySphere
    {
    public:
        /**
         * Construct an AABB from a centre point and radius.
         * @param centre Centre of the sphere, in no particular space.
         * @param radius Radius of the sphere (distance from centre to edge).
         */
        BoundarySphere(tz::Vec3 centre, float radius);
        /**
         * Query as to whether a point is within the current sphere.
         * @param point Point in 3D space to query is within the current sphere.
         * @return True if the point is bound within the current sphere. False otherwise.
         */
        bool collides(const tz::Vec3& point) const;
        /**
         * Query as to whether a given sphere collides with the current sphere.
         * @param sphere Sphere to query is intersecting with the current sphere.
         * @return True if the spheres intersect. False otherwise.
         */
        bool collides(const BoundarySphere& sphere) const;
        /**
         * Query as to whether a given AABB collides with the current sphere.
         * @param box AABB to query is intersecting with the current sphere.
         * @param True if the box and sphere intersect. False otherwise.
         */
        bool collides(const BoundaryAABB& box) const;
        const tz::Vec3& get_centre() const;
        void set_centre(tz::Vec3 centre);
        float get_radius() const;
    private:
        tz::Vec3 centre;
        float radius;
    };
}

#endif // TOPAZ_GEO_SPHERE_HPP