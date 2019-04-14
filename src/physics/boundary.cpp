#include "physics/boundary.hpp"

BoundingSphere::BoundingSphere(Vector3F centre, float radius): Sphere(radius), centre(centre){}

const Vector3F& BoundingSphere::get_centre() const
{
	return this->centre;
}

bool BoundingSphere::intersects(const Vector3F& point) const
{
    return (this->centre - point).length() < this->radius;
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

bool BoundingSphere::intersects(const AABB &rhs) const
{
    auto dsquared = static_cast<float>(std::pow(this->radius, 2));
    if(this->centre.x < rhs.get_minimum().x)
        dsquared -= std::pow(this->centre.x - rhs.get_minimum().x, 2);
    else if(this->centre.x > rhs.get_maximum().x)
        dsquared -= std::pow(this->centre.x - rhs.get_maximum().x, 2);

    if(this->centre.y < rhs.get_minimum().y)
        dsquared -= std::pow(this->centre.y - rhs.get_minimum().y, 2);
    else if(this->centre.y > rhs.get_maximum().y)
        dsquared -= std::pow(this->centre.y - rhs.get_maximum().y, 2);

    if(this->centre.z < rhs.get_minimum().z)
        dsquared -= std::pow(this->centre.z - rhs.get_minimum().z, 2);
    else if(this->centre.z > rhs.get_maximum().z)
        dsquared -= std::pow(this->centre.z - rhs.get_maximum().z, 2);
    return dsquared > 0;
}

bool BoundingSphere::intersects(const BoundingPlane& rhs) const
{
    return rhs.intersects(*this);
}

bool BoundingSphere::intersects(const BoundingLine& rhs) const
{
    Vector3F q = this->centre - rhs.offset;
    float c = q.length();
    float v = q.dot(rhs.direction);
    return (std::pow(this->radius, 2) - (std::pow(c, 2) - std::pow(v, 2))) >= 0.0f;
}

bool BoundingSphere::intersects(const BoundingPyramidalFrustum &rhs) const
{
    for(const BoundingPlane& plane : rhs.get_planes())
        if(!this->intersects(plane))
            return false;
    return true;
}

bool BoundingSphere::intersects(const BoundaryCluster& rhs) const
{
    return rhs.intersects(*this);
}

AABB::AABB(Vector3F minimum, Vector3F maximum): Cuboid(maximum - minimum), minimum(minimum), maximum(maximum)
{
    this->validate();
}

const Vector3F& AABB::get_minimum() const
{
	return this->minimum;
}

const Vector3F& AABB::get_maximum() const
{
	return this->maximum;
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

bool AABB::intersects(const BoundingSphere& rhs) const
{
    return rhs.intersects(*this);
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

bool AABB::intersects(const BoundingPlane &rhs) const
{
    Vector3F centre = (this->get_maximum() + this->get_minimum()) / 2.0f;
    Vector3F extents = this->get_maximum() - centre;

    float projection_interval_radius = extents.x * std::abs(rhs.get_normal().x) + extents.y * std::abs(rhs.get_normal().y) + extents.z * std::abs(rhs.get_normal().z);

    float box_centre_plane_distance = rhs.get_normal().dot(centre) - rhs.get_distance();
    return std::abs(box_centre_plane_distance) <= projection_interval_radius;
}

bool AABB::intersects(const BoundingPyramidalFrustum &rhs) const
{
    for(const BoundingPlane& plane : rhs.get_planes())
        if(!this->intersects(plane))
            return false;
    return true;
}

bool AABB::intersects(const BoundingLine& rhs) const
{
    auto dir = rhs.direction;
    Vector3F line_normal{1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z};
    float t1 = (this->get_minimum().x - rhs.offset.x) * line_normal.x;
    float t2 = (this->get_maximum().x - rhs.offset.x) * line_normal.x;
    float t3 = (this->get_minimum().y - rhs.offset.y) * line_normal.y;
    float t4 = (this->get_maximum().y - rhs.offset.y) * line_normal.y;
    float t5 = (this->get_minimum().z - rhs.offset.z) * line_normal.z;
    float t6 = (this->get_maximum().z - rhs.offset.z) * line_normal.z;
    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));
    return tmax >= 0 && tmin <= tmax;
}

bool AABB::intersects(const BoundaryCluster& rhs) const
{
    return rhs.intersects(*this);
}

bool AABB::contains(const AABB& rhs) const
{
    return this->minimum <= rhs.get_minimum() && this->maximum >= rhs.get_maximum();
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
    AABB box{(rhs * minimum_homogeneous).xyz(), (rhs * maximum_homogeneous).xyz()};
    box.validate();
    return box;
}

void AABB::validate()
{
    if(minimum.x > maximum.x)
        std::swap(minimum.x, maximum.x);
    if(minimum.y > maximum.y)
        std::swap(minimum.y, maximum.y);
    if(minimum.z > maximum.z)
        std::swap(minimum.z, maximum.z);
}

BoundingPlane::BoundingPlane(Vector3F normal, float distance): Plane(normal), distance(distance){}

BoundingPlane::BoundingPlane(Vector3F a, Vector3F b, Vector3F c): Plane((b - a).cross(c - a).normalised()), distance(-this->normal.dot(a)) {}

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

bool BoundingPlane::intersects(const Vector3F& rhs) const
{
    return this->distance_from(rhs) == 0.0f;
}

bool BoundingPlane::intersects(const BoundingSphere& rhs) const
{
/*	Returns true if |dot product of the plane distance + the sphere centre| is less than the radius of the sphere
//  	  **/
//  	 * / *
//        /**
	return (std::fabs(this->normal.dot(rhs.get_centre()) + this->distance) - rhs.get_radius()) < 0;
}

bool BoundingPlane::intersects(const AABB& rhs) const
{
    return rhs.intersects(*this);
}

bool BoundingPlane::intersects(const BoundingPlane& rhs) const
{
    // Planes intersects only if they're not parallel. I.e when their dot product is not zero.
    return this->get_normal().dot(rhs.get_normal()) != 0.0f;
}

bool BoundingPlane::intersects(const BoundingPyramidalFrustum& rhs) const
{
    for(const BoundingPlane& plane : rhs.get_planes())
        if(!this->intersects(plane))
            return false;
    return true;
}

bool BoundingPlane::intersects(const BoundingLine& rhs) const
{
    // if plane normal and line direction are perpendicular then they will never intersect
    return rhs.direction.dot(this->get_normal()) != 0;
}

bool BoundingPlane::intersects(const BoundaryCluster& rhs) const
{
    return rhs.intersects(*this);
}

BoundingPyramidalFrustum::BoundingPyramidalFrustum(Vector3F camera_position, Vector3F camera_view, float fov, float aspect_ratio, float near_clip, float far_clip): camera_position(camera_position), camera_view(camera_view), fov(fov), aspect_ratio(aspect_ratio), near_clip(near_clip), far_clip(far_clip), near_plane_size(), far_plane_size()
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

BoundingPyramidalFrustum::BoundingPyramidalFrustum(const Camera& camera, float aspect_ratio): BoundingPyramidalFrustum(camera.position, camera.rotation, camera.fov, aspect_ratio, camera.near_clip, camera.far_clip){}

const BoundingPyramidalFrustum::BoundingPlaneSextet& BoundingPyramidalFrustum::get_planes() const
{
    return this->planes;
}

bool BoundingPyramidalFrustum::intersects(const Vector3F& point) const
{
    for(const BoundingPlane& plane : this->planes)
    {
        if(plane.distance_from(point) < 0.0f)
            return false;
    }
    return true;
}

bool BoundingPyramidalFrustum::intersects(const BoundingSphere& rhs) const
{
    return rhs.intersects(*this);
}

bool BoundingPyramidalFrustum::intersects(const AABB& rhs) const
{
    return rhs.intersects(*this);
}

bool BoundingPyramidalFrustum::intersects(const BoundingPlane& rhs) const
{
    return rhs.intersects(*this);
}

bool BoundingPyramidalFrustum::intersects([[maybe_unused]] const BoundingPyramidalFrustum& rhs) const
{
    // TODO: Implement using the following choices of algorithms:
    // V-Clip Algorithm (https://dl.acm.org/citation.cfm?id=285860)
    // Separating Axis Theorem (will be painful with testing two frusta) (https://gamedevelopment.tutsplus.com/tutorials/collision-detection-using-the-separating-axis-theorem--gamedev-169)
    return false;
}

bool BoundingPyramidalFrustum::intersects([[maybe_unused]] const BoundingLine& rhs) const
{
    // TODO: Implement when not so tired.
    return false;
}

bool BoundingPyramidalFrustum::intersects(const BoundaryCluster& rhs) const
{
    return rhs.intersects(*this);
}

bool BoundingPyramidalFrustum::contains(const AABB& box) const
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

BoundingLine::BoundingLine(Vector3F offset, Vector3F direction): offset(offset), direction(direction){}

bool BoundingLine::intersects(const Vector3F& point) const
{
    // line.p + line.d(?) = p
    // amount = (point - line.p) / line.d
    Vector3F delta = point - this->offset;
    float ax = delta.x / this->direction.x;
    float ay = delta.y / this->direction.y;
    float az = delta.z / this->direction.z;
    return ax != ay || ay != az || ax != az;
}

bool BoundingLine::intersects(const BoundingSphere& rhs) const
{
    return rhs.intersects(*this);
}

bool BoundingLine::intersects(const AABB& rhs) const
{
    return rhs.intersects(*this);
}

bool BoundingLine::intersects(const BoundingPlane& rhs) const
{
    return rhs.intersects(*this);
}

bool BoundingLine::intersects(const BoundingPyramidalFrustum& rhs) const
{
    return rhs.intersects(*this);
}

bool BoundingLine::intersects(const BoundingLine& rhs) const
{
    return rhs.intersects(*this);
}

bool BoundingLine::intersects(const BoundaryCluster& rhs) const
{
    return rhs.intersects(*this);
}

BoundaryCluster::BoundaryCluster(std::vector<BoundaryCluster::ClusterComponent>&& components): components(std::move(components)){}
BoundaryCluster::BoundaryCluster(): components(){}

BoundaryCluster::BoundaryCluster(const BoundaryCluster& copy): components()
{
    for(const auto& [integration, boundary_ptr] : copy.components)
    {
        auto sphere_component = dynamic_cast<BoundingSphere*>(boundary_ptr.get());
        auto box_component = dynamic_cast<AABB*>(boundary_ptr.get());
        auto plane_component = dynamic_cast<BoundingPlane*>(boundary_ptr.get());
        auto frustum_component = dynamic_cast<BoundingPyramidalFrustum*>(boundary_ptr.get());
        auto cluster_component = dynamic_cast<BoundaryCluster*>(boundary_ptr.get());
        std::unique_ptr<Boundary> payload = nullptr;
        if(sphere_component != nullptr)
            payload = std::make_unique<BoundingSphere>(*sphere_component);
        else if(box_component != nullptr)
            payload = std::make_unique<AABB>(*box_component);
        else if(plane_component != nullptr)
            payload = std::make_unique<BoundingPlane>(*plane_component);
        else if(frustum_component != nullptr)
            payload = std::make_unique<BoundingPyramidalFrustum>(*frustum_component);
        else if(cluster_component != nullptr)
            payload = std::make_unique<BoundaryCluster>(*cluster_component);
        this->components.push_back(std::make_pair(integration, std::move(payload)));
    }
}

BoundaryCluster::BoundaryCluster(BoundaryCluster&& move): components(std::move(move.components))
{
    move.components.clear();
}

BoundaryCluster& BoundaryCluster::operator=(BoundaryCluster rhs)
{
    BoundaryCluster::swap(*this, rhs);
    return *this;
}

BoundaryCluster& BoundaryCluster::operator=(BoundaryCluster&& rhs)
{
    this->components = std::move(rhs.components);
    return *this;
}

std::size_t BoundaryCluster::get_cluster_size() const
{
    return this->components.size();
}

std::optional<BoundaryCluster::NonOwningClusterComponent> BoundaryCluster::get_boundary_at_index(std::size_t index) const
{
    if(index >= this->get_cluster_size())
        return std::nullopt;
    const BoundaryCluster::ClusterComponent& this_component = this->components[index];
    return {BoundaryCluster::NonOwningClusterComponent{this_component.first, *this_component.second}};
}

bool BoundaryCluster::set_boundary_at_index(std::size_t index, BoundaryCluster::ClusterIntegration integration, std::unique_ptr<Boundary> boundary)
{
    bool needed_resizing = false;
    while(this->get_cluster_size() <= index)
    {
        this->components.emplace_back(BoundaryCluster::ClusterIntegration::UNION, nullptr);
        needed_resizing = true;
    }
    this->components[index].first = integration;
    this->components[index].second = std::move(boundary);
    return needed_resizing;
}

bool BoundaryCluster::intersects(const Vector3F& point) const
{
    return this->intersects_impl<Vector3F>(point);
}

bool BoundaryCluster::intersects(const BoundingSphere& rhs) const
{
    return this->intersects_impl<BoundingSphere>(rhs);
}

bool BoundaryCluster::intersects(const AABB& rhs) const
{
    return this->intersects_impl<AABB>(rhs);
}

bool BoundaryCluster::intersects(const BoundingPlane& rhs) const
{
    return this->intersects_impl<BoundingPlane>(rhs);
}

bool BoundaryCluster::intersects(const BoundingPyramidalFrustum& rhs) const
{
    return this->intersects_impl<BoundingPyramidalFrustum>(rhs);
}

bool BoundaryCluster::intersects(const BoundingLine& rhs) const
{
    return this->intersects_impl<BoundingLine>(rhs);
}

bool BoundaryCluster::intersects(const BoundaryCluster& rhs) const
{
    bool current_fail_flag = false;
    for(const auto& [integration, boundary_ptr] : this->components)
    {
        switch(integration)
        {
            case BoundaryCluster::ClusterIntegration::INTERSECTION:
                current_fail_flag = current_fail_flag && boundary_ptr->intersects(rhs);
                break;
            case BoundaryCluster::ClusterIntegration::UNION:
                current_fail_flag = current_fail_flag || boundary_ptr->intersects(rhs);
                break;
        }
    }
    return current_fail_flag;
}

void BoundaryCluster::swap(BoundaryCluster& a, BoundaryCluster& b)
{
    std::swap(a.components, b.components);
}