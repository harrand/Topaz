#ifndef BOUNDARY_HPP
#define BOUNDARY_HPP
#include "graphics/camera.hpp"

/**
* Used to bound physical spherical shapes in 3D space.
*/
class BoundingSphere
{
public:
	/**
	 * Construct a Bounding Sphere with a 3-dimensional origin and a radius.
	 * @param centre - The desired origin of the sphere.
	 * @param radius - The desired radius of the sphere.
	 */
	BoundingSphere(Vector3F centre, float radius);

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
private:
	/// Centre of the sphere, in world-space.
	Vector3F centre;
	/// Radius of the sphere.
	float radius;
};

class AABB
{
public:
	/**
	 * Construct an AABB from a minimum and maximum point in 3-dimensional space.
	 * @param minimum - The minimum point
	 * @param maximum - The maximum point
	 */
	AABB(Vector3F minimum, Vector3F maximum);

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
	AABB expand_to(const AABB& other) const;
    AABB operator*(const Matrix4x4& rhs) const;
private:
	/// Minimum 3-dimensional position of the AABB, in world-space.
	Vector3F minimum;
	/// Maximum 3-dimensional positon of the AABB, in world-space.
	Vector3F maximum;
};

/**
* Used to represent planes in 3-dimensional space. Useful for objects such as walls or floors. Also used to comprise a Frustum.
*/
class BoundingPlane
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
private:
	/// The normal Vector to the plane.
	Vector3F normal;
	/// Geometric distance from the plane to the origin [0, 0, 0] in world-space.
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
    /**
	 * Query whether an AABB is inside this frustum.
	 * @param box - The AABB to query whether is contained in this frustum.
	 * @return - True if the box is in this frustum. False otherwise.
	 */
    bool contains(const AABB& box) const;
private:
	/// Position of the camera, in world-space.
	Vector3F camera_position;
	/// Directional vector representing camera view.
	Vector3F camera_view;
	/// Field-of-view in radians.
	float fov;
	/// Aspect-ratio of the camera's perspective.
	float aspect_ratio;
	/// Minimum distance from the camera for objects to render.
	float near_clip;
	/// Maximum distance from the camera for objects to render.
	float far_clip;
	/// 2-dimensional size of the near-plane.
	Vector2F near_plane_size;
	/// 2-dimensional size of the far-plane.
	Vector2F far_plane_size;
	/// Frustum is contrained by six planes. Plane array format: top, bottom, left, right, near, far.
	std::array<BoundingPlane, 6> planes;
};

#endif