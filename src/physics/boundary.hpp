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
	BoundingSphere(Vector3F centre, float radius);
	BoundingSphere(const BoundingSphere& copy) = default;
	BoundingSphere(BoundingSphere&& move) = default;
	~BoundingSphere() = default;
	BoundingSphere& operator=(const BoundingSphere& rhs) = default;

	const Vector3F& get_centre() const;
	float get_radius() const;
	bool intersects(const BoundingSphere& rhs) const;
	virtual bool intersects(Boundary* other_boundary) const override;
private:
	const Vector3F centre;
	const float radius;
};

/**
* Axis-Aligned Bounding-Box. Very lightweight but is a very limited and minimalistic box-shaped boundary for an object.
* Use if performance > precision.
*/
class AABB : public Boundary
{
public:
	AABB(Vector3F minimum, Vector3F maximum);
	AABB(const AABB& copy) = default;
	AABB(AABB&& move) = default;
	~AABB() = default;
	AABB& operator=(const AABB& rhs) = default;
	
	const Vector3F& get_minimum() const;
	const Vector3F& get_maximum() const;
	bool intersects(const AABB& rhs) const;
	bool intersects(const Vector3F& point) const;
	virtual bool intersects(Boundary* other_boundary) const override;
private:
	const Vector3F minimum, maximum;
};

/**
* Used to bound planes. Useful for objects such as walls or floors.
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
	BoundingPlane(Vector3F a, Vector3F b, Vector3F c);
	BoundingPlane(const BoundingPlane& copy) = default;
	BoundingPlane(BoundingPlane&& move) = default;
	~BoundingPlane() = default;
	BoundingPlane& operator=(const BoundingPlane& rhs) = default;

	const Vector3F& get_normal() const;
	float get_distance() const;
    float distance_from(const Vector3F& point) const;
	BoundingPlane normalised() const;
	bool intersects(const BoundingSphere& other) const;
	virtual bool intersects(Boundary* other_boundary) const override;
private:
	Vector3F normal;
	float distance;
};

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
    bool contains(const Vector3F& point) const;
private:
	Vector3F camera_position, camera_view;
	float fov, aspect_ratio, near_clip, far_clip;
	Vector2F near_plane_size, far_plane_size;
	/// Plane array format: top, bottom, left, right, near, far.
	std::array<BoundingPlane, 6> planes;
};

#endif