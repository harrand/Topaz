#include "physics/boundary.hpp"

BoundingSphere::BoundingSphere(Vector3F centre, float radius): centre(centre), radius(radius){}

const Vector3F& BoundingSphere::get_centre() const
{
	return this->centre;
}

float BoundingSphere::get_radius() const
{
	return this->radius;
}

bool BoundingSphere::intersects(const BoundingSphere& rhs) const
{
	/* returns true if the total radii is less than the distance between their centres
		** **
	   *  *  *
	    ** **
	*/
	float radius_distance = this->radius + rhs.get_radius();
	float centre_distance = (this->centre - rhs.get_centre()).length();
	return centre_distance < radius_distance;
}

AABB::AABB(Vector3F minimum, Vector3F maximum): minimum(minimum), maximum(maximum){}

const Vector3F& AABB::get_minimum() const
{
	return this->minimum;
}

const Vector3F& AABB::get_maximum() const
{
	return this->maximum;
}

bool AABB::intersects(const AABB& rhs) const
{
	/* Returns true if the maximum distance between the boxes in any of the three dimensions is less than zero (aka there is no distance)
		*---*
		|  *|--*
		*--|*  |
		   *---*
	*/
    if(this->maximum.x < rhs.minimum.x || this->minimum.x > rhs.maximum.x) return false;
    if(this->maximum.y < rhs.minimum.y || this->minimum.y > rhs.maximum.y) return false;
    return !(this->maximum.z < rhs.minimum.z || this->minimum.z > rhs.maximum.z);
}

bool AABB::intersects(const Vector3F& point) const
{
	/* Returns true if the point is greater than the minimum but less than the maximum in all three dimensions
		*---*
		| * |
		*---*
	*/
	bool meet_x = this->minimum.x <= point.x && this->maximum.x >= point.x;
	bool meet_y = this->minimum.y <= point.y && this->maximum.y >= point.y;
	bool meet_z = this->minimum.z <= point.z && this->maximum.z >= point.z;
	return meet_x && meet_y && meet_z;
}

AABB AABB::expand_to(const AABB &other) const
{
    Vector3F min = this->minimum, max = this->maximum;
    min.x = std::min(min.x, other.minimum.x);
    min.y = std::min(min.y, other.minimum.y);
    min.z = std::min(min.z, other.minimum.z);

    max.x = std::max(max.x, other.maximum.x);
    max.y = std::max(max.y, other.maximum.y);
    max.z = std::max(max.z, other.maximum.z);
    return {min, max};
}

AABB AABB::operator*(const Matrix4x4& rhs) const
{
    Vector4F minimum_homogeneous = {this->minimum, 1.0f};
    Vector4F maximum_homogeneous = {this->maximum, 1.0f};
    return {(rhs * minimum_homogeneous).xyz(), (rhs * maximum_homogeneous).xyz()};
}

BoundingPlane::BoundingPlane(Vector3F normal, float distance): normal(normal), distance(distance){}

BoundingPlane::BoundingPlane(Vector3F a, Vector3F b, Vector3F c): normal((b - a).cross(c - a).normalised()), distance(-this->normal.dot(a)) {}

const Vector3F& BoundingPlane::get_normal() const
{
	return this->normal;
}

float BoundingPlane::get_distance() const
{
	return this->distance;
}

float BoundingPlane::distance_from(const Vector3F& point) const
{
    return ((this->normal.x * point.x) + (this->normal.y * point.y) + (this->normal.z * point.z) + this->distance) / this->normal.length();
}

BoundingPlane BoundingPlane::normalised() const
{
	float length = this->normal.length();
	return {this->normal.normalised(), this->distance / length};
}

bool BoundingPlane::intersects(const BoundingSphere& rhs) const
{
/*	Returns true if |dot product of the plane distance + the sphere centre| is less than the radius of the sphere
//  	  **/
//  	 * / *
//        /**
	return (std::fabs(this->normal.dot(rhs.get_centre()) + this->distance) - rhs.get_radius()) < 0;
}

Frustum::Frustum(Vector3F camera_position, Vector3F camera_view, float fov, float aspect_ratio, float near_clip, float far_clip): camera_position(camera_position), camera_view(camera_view), fov(fov), aspect_ratio(aspect_ratio), near_clip(near_clip), far_clip(far_clip), near_plane_size(), far_plane_size()
{
	this->near_plane_size.x = 2.0f * std::tan(this->fov / 2.0f) * this->near_clip;
	this->near_plane_size.y = this->near_plane_size.x * this->aspect_ratio;
	this->far_plane_size.x = 2.0f * std::tan(this->fov / 2.0f) * this->far_clip;
	this->far_plane_size.y = this->far_plane_size.x * this->aspect_ratio;
	Camera temporary_camera(this->camera_position, this->camera_view, this->fov, this->near_clip, this->far_clip);
	/// Camera axes:
	Vector3F X = temporary_camera.right();
    Vector3F Y = temporary_camera.up();
    /// Compute centre of near_plane and far_plane.
    Vector3F near_centre = temporary_camera.position + (temporary_camera.forward() * temporary_camera.near_clip);
    Vector3F far_centre = temporary_camera.position + (temporary_camera.forward() * temporary_camera.far_clip);
    /// Compute corners of near_plane and far_plane
    Vector3F ntl = near_centre + Y * this->near_plane_size.y - X * this->far_plane_size.x;
    Vector3F ntr = near_centre + Y * this->near_plane_size.y + X * this->far_plane_size.x;
    Vector3F nbl = near_centre - Y * this->near_plane_size.y - X * this->near_plane_size.x;
    Vector3F nbr = near_centre - Y * this->near_plane_size.y + X * this->near_plane_size.x;

    Vector3F ftl = far_centre + Y * this->far_plane_size.y - X * this->far_plane_size.x;
    Vector3F ftr = far_centre + Y * this->far_plane_size.y + X * this->far_plane_size.x;
    Vector3F fbl = far_centre - Y * this->far_plane_size.y - X * this->far_plane_size.x;
    Vector3F fbr = far_centre - Y * this->far_plane_size.y + X * this->far_plane_size.x;

    this->planes[0] = {ntr, ntl, ftl};
    this->planes[1] = {nbl, nbr, fbr};
    this->planes[2] = {ntl, nbl, fbl};
    this->planes[3] = {nbr, ntr, fbr};
    this->planes[4] = {ntl, ntr, nbr};
    this->planes[5] = {ftr, ftl, fbl};
}

Frustum::Frustum(const Camera& camera, float aspect_ratio): Frustum(camera.position, camera.rotation, camera.fov, aspect_ratio, camera.near_clip, camera.far_clip){}

bool Frustum::contains(const Vector3F& point) const
{
    for(const BoundingPlane& plane : this->planes)
    {
        if(plane.distance_from(point) < 0.0f)
            return false;
    }
    return true;
}

bool Frustum::contains(const AABB& box) const
{
    float sum = 0.0f;
    for(const BoundingPlane& plane : this->planes)
    {
        if (plane.get_normal().x > 0)
            sum = plane.get_normal().x * box.get_maximum().x;
        else
            sum = plane.get_normal().x * box.get_minimum().x;

        if (plane.get_normal().y > 0)
            sum += plane.get_normal().y * box.get_maximum().y;
        else
            sum += plane.get_normal().y * box.get_minimum().y;

        if (plane.get_normal().z > 0)
            sum += plane.get_normal().z * box.get_maximum().z;
        else
            sum += plane.get_normal().z * box.get_minimum().z;

        if (sum <= -plane.get_distance())
            return false;
    }
    return true;
}