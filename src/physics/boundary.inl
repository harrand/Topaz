#include <type_traits>

template<class BoundaryType, typename... Args>
BoundaryType& BoundaryCluster::emplace(ClusterIntegration integration, Args&&... args)
{
    auto& pair = this->components.emplace_back(integration, std::make_unique<BoundaryType>(std::forward<Args>(args)...));
    return *dynamic_cast<BoundaryType*>(pair.second.get());
}

template<typename... Args>
BoundingSphere& BoundaryCluster::emplace_sphere(BoundaryCluster::ClusterIntegration integration, Args&&... args)
{
    return this->emplace<BoundingSphere>(integration, std::forward<Args>(args)...);
}

template<typename... Args>
AABB& BoundaryCluster::emplace_box(BoundaryCluster::ClusterIntegration integration, Args&&... args)
{
    return this->emplace<AABB>(integration, std::forward<Args>(args)...);
}

template<typename... Args>
BoundingPlane& BoundaryCluster::emplace_plane(BoundaryCluster::ClusterIntegration integration, Args&&... args)
{
    return this->emplace<BoundingPlane>(integration, std::forward<Args>(args)...);
}

template<typename... Args>
BoundingPyramidalFrustum& BoundaryCluster::emplace_frustum(BoundaryCluster::ClusterIntegration integration, Args&&... args)
{
    return this->emplace<BoundingPyramidalFrustum>(integration, std::forward<Args>(args)...);
}

template<typename... Args>
BoundaryCluster& BoundaryCluster::emplace_cluster(BoundaryCluster::ClusterIntegration integration, Args&&... args)
{
    return this->emplace<BoundaryCluster>(integration, std::forward<Args>(args)...);
}

template<class BoundaryType>
bool BoundaryCluster::intersects_impl(const BoundaryType& boundary) const
{
    static_assert(std::is_base_of_v<Boundary, BoundaryType> || std::is_same_v<Vector3F, BoundaryType>, "[Topaz Physics]: BoundaryCluster::intersects_impl(...) has unsupported type. Chances are you have subclassed BoundaryCluster incorrectly.");
    if(this->components.empty())
        return false;
    bool current_fail_flag;
    switch(this->components.front().first)
    {
        default:
        case BoundaryCluster::ClusterIntegration::UNION:
            current_fail_flag = false;
            break;
        case BoundaryCluster::ClusterIntegration::INTERSECTION:
            current_fail_flag = true;
            break;
    }
    for(const auto& [integration, boundary_ptr] : this->components)
    {
        switch(integration)
        {
            case BoundaryCluster::ClusterIntegration::INTERSECTION:
                current_fail_flag = current_fail_flag && boundary_ptr->intersects(boundary);
                break;
            case BoundaryCluster::ClusterIntegration::UNION:
                current_fail_flag = current_fail_flag || boundary_ptr->intersects(boundary);
                break;
        }
    }
    return current_fail_flag;
}