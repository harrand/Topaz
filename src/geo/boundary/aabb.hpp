#ifndef TOPAZ_GEO_BOUNDARY_AABB_HPP
#define TOPAZ_GEO_BOUNDARY_AABB_HPP
#include "core/vector.hpp"

namespace tz::geo
{
    class BoundarySphere;
    /**
	 * \addtogroup tz_geo Topaz Geometry Library (tz::geo)
	 * A collection of geometric data structures and mathematical types, such as vectors and matrices.
	 * @{
	 */

    /**
     * Represents an axis-aligned bounding-box.
     */
    class BoundaryAABB
    {
    public:
        /**
         * Construct an AABB from a range of positions.
         * @param min Position lying within the AABB as close as possible to the origin of the current space.
         * @param max Position lying within the AABB as far as possible to the origin of the current space.
         */
        BoundaryAABB(tz::Vec3 min, tz::Vec3 max);
        tz::Vec3 get_minimum() const;
        tz::Vec3 get_maximum() const;
        void set_minimum(tz::Vec3 min);
        void set_maximum(tz::Vec3 max);
        /**
         * Query as to whether a point is within the current AABB.
         * @param point Point in 3D space to query is within the current AABB.
         * @return True if the point is bound within the current AABB. False otherwise.
         */
        bool collides(const tz::Vec3& point) const;
        /**
         * Query as to whether a given AABB collides with the current AABB.
         * @param box AABB to query is intersecting with the current AABB.
         * @return True if the AABBs intersect. False otherwise.
         */
        bool collides(const BoundaryAABB& box) const;
        /**
         * Query as to whether a given Sphere collides with the current AABB.
         * @param sphere Sphere to query is intersecting with the current AABB.
         */
        bool collides(const BoundarySphere& sphere) const;
    private:
        /// Swaps min and max if |min| > |max|.
        void ensure();
        tz::Vec3 min, max;
    };
    /**
     * @}
     */
}

#endif // TOPAZ_GEO_BOUNDARY_AABB_HPP