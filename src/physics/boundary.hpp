#ifndef BOUNDARY_HPP
#define BOUNDARY_HPP
#include "../camera.hpp"

/**
* Abstract. Not available for non-polymorphic use. Inherit from this to create custom boundaries.
* Represents a simple boundary in space.
*/
class Boundary
{
public:
	Boundary() = default;
	Boundary(const Boundary& copy) = default;
	Boundary(Boundary&& move) = default;
	~Boundary() = default;
	Boundary& operator=(const Boundary& rhs) = default;
	
	/**
	* Pure virtual. Override this if you want to make your own Boundaries.
	*/
	virtual bool intersects(Boundary* other_boundary) const = 0;
};

/**
* Used to bound physical spherical shapes in 3D space.
*/
class BoundingSphere : public Boundary
{
public:
	/**
	 * Construct a Bounding Sphere with a 3-dimensional origin and a radius.
	 * @param centre - The desired origin of the sphere.
	 * @param radius - The desired radius of the sphere.
	 */
	BoundingSphere(Vector3F centre, float radius);
	BoundingSphere(const BoundingSphere& copy) = default;
	BoundingSphere(BoundingSphere&& move) = default;
	~BoundingSphere() = default;
	BoundingSphere& operator=(const BoundingSphere& rhs) = default;

	/**
	 * Retrieve the centre of the sphere.
	 * @return - Origin of the sphere.
	 */
	const Vector3F& get_centre() const;
	/**
	 * Retrieve the radius of the sphere.
	 * @return - Radius of the sphere.
	 */
	float get_radius() const;
	/**
	 * Query whether this sphere intersects another sphere.
	 * @param rhs - The other BoundingSphere to query whether intersects with this sphere.
	 * @return - True if the spheres intersect. False otherwise.
	 */
	bool intersects(const BoundingSphere& rhs) const;
	/**
	 * Checks whether this BoundingSphere intersects the polymorphic boundary provided.
	 * @param other_boundary - The polymorphic boundary to check whether this intersects with.
	 * @return - True if this BoundingSphere intersects the boundary. False otherwise.
	 */
	virtual bool intersects(Boundary* other_boundary) const override;
private:
	Vector3F centre;
	float radius;
};

/**
* Axis-Aligned Bounding-Box. Very lightweight but is a very limited and minimalistic box-shaped boundary for an object.
* Use if performance > precision.
*/
class AABB : public Boundary
{
public:
	/**
	 * Construct an AABB from a minimum and maximum point in 3-dimensional space.
	 * @param minimum - The minimum point
	 * @param maximum - The maximum point
	 */
	AABB(Vector3F minimum, Vector3F maximum);
	AABB(const AABB& copy) = default;
	AABB(AABB&& move) = default;
	~AABB() = default;
	AABB& operator=(const AABB& rhs) = default;

	/**
	 * Retrieve the minimum of this AABB.
	 * @return - The minimum point in 3-dimensional space.
	 */
	const Vector3F& get_minimum() const;
	/**
	 * Retrieve the maximum of this AABB.
	 * @return - The maximum point in 3-dimensional space.
	 */
	const Vector3F& get_maximum() const;
	/**
	 * Checks whether this AABB intersects another AABB.
	 * @param rhs - The other AABB to check whether this intersects with.
	 * @return - True if this AABB intersects the parameter. False otherwise.
	 */
	bool intersects(const AABB& rhs) const;
	/**
	 * Checks whether this AABB contains a 3-dimensional point.
	 * @param point - The 3-dimensional point to query whether is contained or not.
	 * @return - True if the parameter is in the AABB. False otherwose.
	 */
	bool intersects(const Vector3F& point) const;
	/**
	 * Checks whether this AABB intersects the polymorphic boundary provided.
	 * @param other_boundary - The polymorphic boundary to check whether this intersects with.
	 * @return - True if this AABB intersects the boundary. False otherwise.
	 */
	virtual bool intersects(Boundary* other_boundary) const override;
private:
	Vector3F minimum, maximum;
};

/**
* Used to represent planes in 3-dimensional space. Useful for objects such as walls or floors. Also used to comprise a Frustum.
*/
class BoundingPlane : public Boundary
{
public:
    /**
     * Construct a BoundingPlane from a normal and distance from origin.
     * Defaults are provided only due to use in BoundingFrustum.
     * @param normal - Normal of the plane
     * @param distance - Distance from the origin
     */
	BoundingPlane(Vector3F normal = {}, float distance = 0.0f);
	/**
	 * Construct a BoundingPlane from three points which the plane should contain.
	 * @param a - One point in 3-dimensional space that belongs on the plane.
	 * @param b - Another point in 3-dimensional space that belongs on the plane.
	 * @param c - Another point in 3-dimensional space that belongs on the plane.
	 */
	BoundingPlane(Vector3F a, Vector3F b, Vector3F c);
	BoundingPlane(const BoundingPlane& copy) = default;
	BoundingPlane(BoundingPlane&& move) = default;
	~BoundingPlane() = default;
	BoundingPlane& operator=(const BoundingPlane& rhs) = default;

	/**
	 * Retrieve the 3-dimensional normal vector of the plane.
	 * @return - The normal of the plane.
	 */
	const Vector3F& get_normal() const;
	/**
	 * Get geometric distance from the plane to the origin [0, 0, 0]
	 * @return - Geometric distance to the origin.
	 */
	float get_distance() const;
	/**
	 * Get geometric distance from the plane to a 3-dimensional point. Can be negative or positive, depending on which side of the plane the point lies.
	 * @param point - The 3-dimensional point to query distance from the plane.
	 * @return - Geometric distance from the plane to the point.
	 */
    float distance_from(const Vector3F& point) const;
	/**
	 * Normalise the normal-vector of the plane, and return a copy of the result.
	 * @return - This plane, but with the normal-vector normalised.
	 */
	BoundingPlane normalised() const;
	/**
	 * Checks whether this plane intersects with a sphere.
	 * @param other - The BoundingSphere to query whether this intersects.
	 * @return - True if this plane intersects the sphere. False otherwise.
	 */
	bool intersects(const BoundingSphere& other) const;
	/**
	 * Checks whether this BoundingPlane intersects the polymorphic boundary provided.
	 * @param other_boundary - The polymorphic boundary to check whether this intersects with.
	 * @return - True if this BoundingPlane intersects the boundary. False otherwise.
	 */
	virtual bool intersects(Boundary* other_boundary) const override;
private:
	Vector3F normal;
	float distance;
};

/**
 * Represent a Frustum-shape to model a perspective-projection matrix.
 * Does not currently support orthographic projection matrix.
 */
class Frustum
{
public:
	/**
	 * Construct a Bounding Frustum from attributes of a perspective matrix.
	 */
	Frustum(Vector3F camera_position, Vector3F camera_view, float fov, float aspect_ratio, float near_clip, float far_clip);
	/**
	 * Construct a Bounding Frustum directly from a camera.
	 * Note: Camera::has_perspective_projection has no effect here; we always assume we're using perspective projection.
	 */
	Frustum(const Camera& camera, float aspect_ratio);
	/**
	 * Query whether a 3-dimensional point is inside this frustum.
	 * @param point - The 3-dimensional point to query whether is contained in this frustum.
	 * @return - True if the point is in this frustum. False otherwise.
	 */
    bool contains(const Vector3F& point) const;
private:
	Vector3F camera_position, camera_view;
	float fov, aspect_ratio, near_clip, far_clip;
	Vector2F near_plane_size, far_plane_size;
	/// Plane array format: top, bottom, left, right, near, far.
	std::array<BoundingPlane, 6> planes;
};

#endif