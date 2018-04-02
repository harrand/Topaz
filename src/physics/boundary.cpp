#include "boundary.hpp"

BoundingSphere::BoundingSphere(Vector3F centre, float radius): Boundary(), centre(centre), radius(radius){}

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

bool BoundingSphere::intersects(Boundary* other_boundary) const
{
	if(dynamic_cast<BoundingSphere*>(other_boundary) == nullptr)
		return false;
	return this->intersects(*dynamic_cast<BoundingSphere*>(other_boundary));
}

AABB::AABB(Vector3F minimum, Vector3F maximum): Boundary(), minimum(minimum), maximum(maximum){}

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
	Vector3F forward_distance = rhs.get_minimum() - this->maximum;
	Vector3F backward_distance = this->minimum - rhs.get_maximum();
	Vector3F distance = std::max(forward_distance, backward_distance);
	return std::max({distance.x, distance.y, distance.z}) < 0;
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

bool AABB::intersects(Boundary* other_boundary) const
{
	if(dynamic_cast<AABB*>(other_boundary) == nullptr)
		return false;
	return this->intersects(*dynamic_cast<AABB*>(other_boundary));
}

BoundingPlane::BoundingPlane(Vector3F normal, float distance): Boundary(), normal(normal), distance(distance){}

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

bool BoundingPlane::intersects(Boundary* other_boundary) const
{
	if(dynamic_cast<BoundingSphere*>(other_boundary) == nullptr)
		return false;
	return this->intersects(*dynamic_cast<BoundingSphere*>(other_boundary));
}

Frustum::Frustum(Vector3F camera_position, Vector3F camera_view, float fov, float aspect_ratio, float near_clip, float far_clip): camera_position(camera_position), camera_view(camera_view), fov(fov), aspect_ratio(aspect_ratio), near_clip(near_clip), far_clip(far_clip), near_plane_size(), far_plane_size()
{
	this->near_plane_size.x = 2.0f * std::tan(this->fov / 2.0f) * this->near_clip;
	this->near_plane_size.y = this->near_plane_size.x * this->aspect_ratio;
	this->far_plane_size.x = 2.0f * std::tan(this->fov / 2.0f) * this->far_clip;
	this->far_plane_size.y = this->far_plane_size.x * this->aspect_ratio;
	Camera temporary_camera(this->camera_position, this->camera_view, this->fov, this->near_clip, this->far_clip);
	/// Camera axes:
	Vector3F Z = temporary_camera.backward();
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