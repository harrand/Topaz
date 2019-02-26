#ifndef BOUNDARY_HPP
#define BOUNDARY_HPP
#include "graphics/camera.hpp"
#include "utility/geometry.hpp"

class BoundingSphere;
class AABB;
class BoundingPlane;
class BoundingPyramidalFrustum;
class BoundaryCluster;
class BoundingLine;

class Boundary
{
public:
    virtual bool intersects(const Vector3F& rhs) const = 0;
    virtual bool intersects(const BoundingSphere& rhs) const = 0;
    virtual bool intersects(const AABB& rhs) const = 0;
    virtual bool intersects(const BoundingPlane& rhs) const = 0;
    virtual bool intersects(const BoundingPyramidalFrustum& rhs) const = 0;
    virtual bool intersects(const BoundaryCluster& rhs) const = 0;
    virtual bool intersects(const BoundingLine& rhs) const = 0;
};
/**
* Used to bound physical spherical shapes in 3D space.
*/
class BoundingSphere : public Sphere, public Boundary
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
	 * Query whether this sphere intersects (contains) a point.
	 * @param point - The 3D position vector representing the given point
	 * @return - True if the sphere contains the point. False otherwise
	 */
	virtual bool intersects(const Vector3F& point) const override;
	/**
	 * Query whether this sphere intersects another sphere.
	 * @param rhs - The other BoundingSphere to query whether intersects with this sphere
	 * @return - True if the spheres intersect. False otherwise
	 */
	virtual bool intersects(const BoundingSphere& rhs) const override;
	/**
	 * Query whether this sphere intersects a box.
	 * @param rhs - The box to query whether intersects with this sphere
	 * @return - True if the sphere intersecs the box. False otherwise
	 */
	virtual bool intersects(const AABB& rhs) const override;
	/**
	 * Query whether this sphere intersects a given infinite plane.
	 * @param rhs - The plane to query whether intersects with this sphere
	 * @return - True if the plane intersects this sphere. False otherwise
	 */
	virtual bool intersects(const BoundingPlane& rhs) const override;
	/**
	 * Query whether this sphere intersects a given pyramidal frustum.
	 * @param rhs - The frustum to query whether intersects with this sphere
	 * @return - True if the frustum intersects this sphere. False otherwise
	 */
	virtual bool intersects(const BoundingPyramidalFrustum& rhs) const override;
	/**
	 * Query whether this sphere intersects a given line.
	 * @param rhs - The line to query whether intersects with this sphere
	 * @return - True if the line intersects this sphere. False otherwise
	 */
	virtual bool intersects(const BoundingLine& rhs) const override;
	/**
	 * Query whether this sphere intersects a given cluster of boundaries.
	 * @param rhs - The cluster of boundary to query whether intersects with this sphere
	 * @return - True if the cluster intersects this sphere. False otherwise
	 */
	virtual bool intersects(const BoundaryCluster& rhs) const override;
private:
	/// Centre of the sphere, in world-space.
	Vector3F centre;
};

class AABB : public Cuboid, public Boundary
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
	 * Checks whether this AABB contains a 3-dimensional point.
	 * @param point - The 3-dimensional point to query whether is contained or not
	 * @return - True if the parameter is in the AABB. False otherwise
	 */
	virtual bool intersects(const Vector3F& point) const override;
	/**
	 * Checks whether this AABB intersects a sphere.
	 * @param rhs - The sphere to check whether this box intersects with
	 * @return - True if this AABB intersects the sphere. False otherwise
	 */
	virtual bool intersects(const BoundingSphere& rhs) const override;
	/**
	 * Checks whether this AABB intersects another AABB.
	 * @param rhs - The other AABB to check whether this intersects with
	 * @return - True if this AABB intersects the parameter. False otherwise
	 */
	virtual bool intersects(const AABB& rhs) const override;
	/**
	 * Checks whether this AABB intersects a plane.
	 * @param rhs - The plane to check whether this box intersects with
	 * @return - True if this AABB intersects the plane. False otherwise
	 */
	virtual bool intersects(const BoundingPlane& rhs) const override;
	/**
	 * Checks whether this AABB intersects a pyramidal frustum.
	 * @param rhs - The pyramidal frustum to check whether this box intersects with
	 * @return - True if this AABB intersects the pyramidal frustum. False otherwise
	 */
	virtual bool intersects(const BoundingPyramidalFrustum& rhs) const override;
	/**
	 * Checks whether this AABB intersects a line.
	 * @param rhs - THe line to check whether this both intersects with
	 * @return - True if this AABB intersects the line. False otherwise
	 */
	virtual bool intersects(const BoundingLine& rhs) const override;
	/**
	 * Checks whether this AABB intersects a cluster of boundaries.
	 * @param rhs - The cluster of boundaries to check whether this box interacts with
	 * @return - True if this AABB intersects the cluster. False otherwise
	 */
	virtual bool intersects(const BoundaryCluster& rhs) const override;
	AABB expand_to(const AABB& other) const;
    AABB operator*(const Matrix4x4& rhs) const;
private:
	void validate();
	/// Minimum 3-dimensional position of the AABB, in world-space.
	Vector3F minimum;
	/// Maximum 3-dimensional positon of the AABB, in world-space.
	Vector3F maximum;
};

/**
* Used to represent planes in 3-dimensional space. Useful for objects such as walls or floors. Also used to comprise a BoundingPyramidalFrustum.
*/
class BoundingPlane : public Plane, public Boundary
{
public:
    /**
     * Construct a BoundingPlane from a normal and distance from origin.
     * Defaults are provided only due to use in BoundingBoundingPyramidalFrustum.
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
	 * Checks whether this plane contains a given point.
	 * @param point - 3D position vector representing the given point
	 * @return - True if the point is encompassed by the plane. False otherwise
	 */
	virtual bool intersects(const Vector3F& point) const override;
	/**
	 * Checks whether this plane intersects with a sphere.
	 * @param other - The BoundingSphere to query whether this intersects with
	 * @return - True if this plane intersects the sphere. False otherwise
	 */
	virtual bool intersects(const BoundingSphere& rhs) const override;
	/**
	 * Checks whether this plane intersects with a box.
	 * @param rhs - The AABB to query whether this intersects with
	 * @return - True if this plane intersects the box. False otherwise
	 */
	virtual bool intersects(const AABB& rhs) const override;
	/**
	 * Checks whether this plane intersects another plane.
	 * @param rhs - The other plane to query whether this intersects with
	 * @return - True if the planes intersect. False otherwise
	 */
	virtual bool intersects(const BoundingPlane& rhs) const override;
	/**
	 * Checks whether this plane intersects with a pyramidal frustum.
	 * @param rhs - The pyramidal frustum to query whether this intersects with
	 * @return - True if the plane intersects the frustum. False otherwise
	 */
	virtual bool intersects(const BoundingPyramidalFrustum& rhs) const override;
	/**
	 * Checks whether this plane intersects with a line.
	 * @param rhs - The line to query whether this intersects with
	 * @return - True if the plane intersects the line. False otherwise
	 */
	virtual bool intersects(const BoundingLine& rhs) const override;
	/**
	 * Checks whether this plane intersects with a cluster of boundaries.
	 * @param rhs - The cluster of boundaries to query whether this intersects with
	 * @return - True if the plane intersects the cluster. False otherwise
	 */
	virtual bool intersects(const BoundaryCluster& rhs) const override;
private:
	using Plane::normal;
	/// Geometric distance from the plane to the origin [0, 0, 0] in world-space.
	float distance;
};

/**
 * Represent a BoundingPyramidalFrustum-shape to model a perspective-projection matrix.
 * Does not currently support orthographic projection matrix.
 */
class BoundingPyramidalFrustum : public Boundary
{
public:
	using BoundingPlaneSextet = std::array<BoundingPlane, 6>;
	/**
	 * Construct a Bounding BoundingPyramidalFrustum from attributes of a perspective matrix.
	 */
	BoundingPyramidalFrustum(Vector3F camera_position, Vector3F camera_view, float fov, float aspect_ratio, float near_clip, float far_clip);
	/**
	 * Construct a Bounding BoundingPyramidalFrustum directly from a camera.
	 * Note: Camera::has_perspective_projection has no effect here; we always assume we're using perspective projection.
	 */
	BoundingPyramidalFrustum(const Camera& camera, float aspect_ratio);
	/**
	 * Get the planes constituting this pyramidal frustum in the following order:
	 * Top, Bottom, Left, Right, Near, Far
	 * @return - Ordered array of planes
	 */
	const BoundingPlaneSextet& get_planes() const;
	/**
	 * Query whether a 3-dimensional point is inside this BoundingPyramidalFrustum.
	 * @param point - The 3-dimensional point to query whether is contained in this BoundingPyramidalFrustum.
	 * @return - True if the point is in this BoundingPyramidalFrustum. False otherwise.
	 */
    virtual bool intersects(const Vector3F& point) const override;
    /**
     * Query whether this pyramidal frustum intersects with a sphere.
     * @param rhs - The given sphere
     * @return - True if the frustum intersects the sphere. False otherwise
     */
    virtual bool intersects(const BoundingSphere& rhs) const override;
	/**
     * Query whether this pyramidal frustum intersects with a box.
     * @param rhs - The given AABB
     * @return - True if the frustum intersects the box. False otherwise
     */
    virtual bool intersects(const AABB& rhs) const override;
	/**
     * Query whether this pyramidal frustum intersects with a plane.
     * @param rhs - The given plane
     * @return - True if the frustum intersects the plane. False otherwise
     */
    virtual bool intersects(const BoundingPlane& rhs) const override;
	/**
     * Query whether this pyramidal frustum intersects with another pyramidal frustum.
     * @param rhs - The given frustum
     * @return - True if the frustum intersects the other. False otherwise
     */
    virtual bool intersects(const BoundingPyramidalFrustum& rhs) const override;
    /**
     * Query whether this pyramidal frustum intersects with a line.
     * @param rhs - THe given line
     * @return - True if the frustum intersects the line. False otherwise
     */
    virtual bool intersects(const BoundingLine& rhs) const override;
    /**
     * Query whether this pyramidal frustum intersects with a given cluster of boundaries.
     * @param rhs - The given cluster of boundaries
     * @return - True if the frustum intersects the cluster. False otherwise
     */
    virtual bool intersects(const BoundaryCluster& rhs) const override;
    /**
	 * Query whether an AABB is inside this BoundingPyramidalFrustum.
	 * @param box - The AABB to query whether is contained in this BoundingPyramidalFrustum.
	 * @return - True if the box is in this BoundingPyramidalFrustum. False otherwise.
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
	/// BoundingPyramidalFrustum is contrained by six planes. Plane array format: top, bottom, left, right, near, far.
	std::array<BoundingPlane, 6> planes;
};

class BoundingLine : public Boundary
{
public:
	BoundingLine(Vector3F offset, Vector3F direction);
	virtual bool intersects(const Vector3F& point) const override;
	virtual bool intersects(const BoundingSphere& rhs) const override;
	virtual bool intersects(const AABB& rhs) const override;
	virtual bool intersects(const BoundingPlane& rhs) const override;
	virtual bool intersects(const BoundingPyramidalFrustum& rhs) const override;
	virtual bool intersects(const BoundingLine& rhs) const override;
	virtual bool intersects(const BoundaryCluster& rhs) const override;

	/// Offset of the line from the origin.
	Vector3F offset;
	/// Directional vector representing the line.
	Vector3F direction;
};

class BoundaryCluster : public Boundary
{
public:
    enum class ClusterIntegration
    {
        UNION, INTERSECTION
    };
    using ClusterComponent = std::pair<ClusterIntegration, std::unique_ptr<Boundary>>;
    using NonOwningClusterComponent = std::pair<ClusterIntegration, const Boundary&>;

    BoundaryCluster(std::vector<ClusterComponent>&& components);
    BoundaryCluster();
    BoundaryCluster(const BoundaryCluster& copy);
    BoundaryCluster(BoundaryCluster&& move);
    BoundaryCluster& operator=(BoundaryCluster rhs);
    BoundaryCluster& operator=(BoundaryCluster&& rhs);

    std::size_t get_cluster_size() const;
    std::optional<NonOwningClusterComponent> get_boundary_at_index(std::size_t index) const;
    bool set_boundary_at_index(std::size_t index, ClusterIntegration integration, std::unique_ptr<Boundary> boundary);
    template<class BoundaryType, typename... Args>
    BoundaryType& emplace(ClusterIntegration integration, Args&&... args);
    template<typename... Args>
    BoundingSphere& emplace_sphere(ClusterIntegration integration, Args&&... args);
    template<typename... Args>
    AABB& emplace_box(ClusterIntegration integration, Args&&... args);
    template<typename... Args>
    BoundingPlane& emplace_plane(ClusterIntegration integration, Args&&... args);
    template<typename... Args>
    BoundingPyramidalFrustum& emplace_frustum(ClusterIntegration integration, Args&&... args);
    template<typename... Args>
    BoundaryCluster& emplace_cluster(ClusterIntegration integration, Args&&... args);

    virtual bool intersects(const Vector3F& point) const override;
    virtual bool intersects(const BoundingSphere& rhs) const override;
    virtual bool intersects(const AABB& rhs) const override;
    virtual bool intersects(const BoundingPlane& rhs) const override;
    virtual bool intersects(const BoundingPyramidalFrustum& rhs) const override;
    virtual bool intersects(const BoundingLine& rhs) const override;
    virtual bool intersects(const BoundaryCluster& rhs) const override;
private:
    template<class BoundaryType>
    bool intersects_impl(const BoundaryType& boundary) const;
    static void swap(BoundaryCluster& a, BoundaryCluster& b);
    std::vector<ClusterComponent> components;
};

#include "physics/boundary.inl"

#endif