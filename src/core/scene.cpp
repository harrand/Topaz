#include "core/scene.hpp"

ScenePartitionNode::ScenePartitionNode(Scene* scene, AABB region, std::vector<Renderable*> enclosed_objects): scene(scene), parent(nullptr), region(region), children({nullptr}), pending_insertion(), enclosed_objects(enclosed_objects), child_mask(0x00), fully_built(false), ready(false){}

void ScenePartitionNode::enqueue_object(Renderable *object)
{
    this->pending_insertion.push(object);
}

const std::vector<std::reference_wrapper<const Renderable>> ScenePartitionNode::get_enclosed_renderables() const
{
    std::vector<std::reference_wrapper<const Renderable>> renderable_crefs;
    for(auto* renderable_ptr : this->enclosed_objects)
        renderable_crefs.push_back(std::cref(*renderable_ptr));
    return renderable_crefs;
}

BoundaryCluster ScenePartitionNode::bound_objects() const
{
    BoundaryCluster cluster;
    for(const auto* renderable : this->enclosed_objects)
        if(renderable->get_boundary().has_value())
            cluster.emplace_box(BoundaryCluster::ClusterIntegration::UNION, renderable->get_boundary().value());
    return cluster;
}

const AABB& ScenePartitionNode::get_region() const
{
    return this->region;
}

const ScenePartitionNode* ScenePartitionNode::get_child(std::size_t child_id) const
{
    return this->children[child_id].get();
}

std::vector<ScenePartitionNode*> ScenePartitionNode::get_children() const
{
    std::vector<ScenePartitionNode*> children;
    for(auto& child_ptr : this->children)
        if(child_ptr != nullptr)
            children.push_back(child_ptr.get());
    return children;
}

bool ScenePartitionNode::has_children() const
{
    for(std::size_t i = 0; i < 8; i++)
        if(this->get_child(i) != nullptr)
            return true;
    return false;
}

const ScenePartitionNode* ScenePartitionNode::get_node_containing(Renderable* object) const
{
    if(object == nullptr)
        return nullptr;
    // Check if this node contains it first.
    auto check_directly = [&](const ScenePartitionNode* node)->const ScenePartitionNode*{for(const auto* renderable_ptr : node->enclosed_objects)if(renderable_ptr == object)return this;return nullptr;};

    if(check_directly(this) != nullptr)
        return this;
    // If not, check each child.
    for(const auto& child_ptr : this->children)
    {
        if(child_ptr == nullptr)
            continue;
        auto container = child_ptr->get_node_containing(object);
        if(container != nullptr)
            return container;
    }
    return nullptr;
}

std::unique_ptr<ScenePartitionNode> ScenePartitionNode::create_node(AABB region, std::vector<Renderable*> enclosed_objects)
{
    if(enclosed_objects.empty())
        return nullptr;
    std::unique_ptr<ScenePartitionNode> child = std::make_unique<ScenePartitionNode>(this->scene, region, enclosed_objects);
    child->parent = this;
    return child;
}

void ScenePartitionNode::find_enclosing_cube()
{
    auto min = scene->get_boundary().get_minimum() * 2.0f;
    auto max = scene->get_boundary().get_maximum() * 2.0f;
    this->region = {min, max};
}

void ScenePartitionNode::update()
{
    if(!this->fully_built)
    {
        while(!this->pending_insertion.empty())
        {
            tz::debug::print("ScenePartitionNode::update(): Inserting Renderable (ID ", this->pending_insertion.front(), ") into unfinished octree...\n");
            this->enclosed_objects.push_back(this->pending_insertion.front());
            this->pending_insertion.pop();
        }
        this->build();
    }
    else
    {
        while(!this->pending_insertion.empty())
        {
            tz::debug::print("ScenePartitionNode::update(): Inserting Renderable (ID ", this->pending_insertion.front(), ") into finished octree...\n");
            this->insert(this->pending_insertion.front());
            this->pending_insertion.pop();
        }
    }
    this->ready = true;
}

void ScenePartitionNode::build()
{
    // If we're a leaf-node, stop recursing.
    if(this->enclosed_objects.empty())
    {
        tz::debug::print("ScenePartitionNode::build(): Invoked, but there are no enclosed objects...\n");
        return;
    }
    Vector3F dimensions = this->region.get_maximum() - this->region.get_minimum();
    if(dimensions.x == 0.0f && dimensions.y == 0.0f && dimensions.z == 0.0f)
    {
        // If our region is empty, do this (?)
        this->find_enclosing_cube();
        dimensions = this->region.get_maximum() - this->region.get_minimum();
        tz::debug::print("ScenePartitionNode::build(): Invoked, but the dimensions of the region were empty, so giving it a default size...\b");
    }

    // Are our dimensions smaller than the minimum allowed size?
    if(dimensions <= tz::scene::minimum_node_size)
        return;

    Vector3F half_dimensions = dimensions / 2.0f;
    Vector3F centre = this->region.get_minimum() + half_dimensions;

    std::array<AABB, 8> octants{AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}};
    // Create subdivided regions for each octant
    octants[0] = {this->region.get_minimum(), centre};
    octants[1] = {Vector3F{centre.x, this->region.get_minimum().y, this->region.get_minimum().z}, Vector3F{this->region.get_maximum().x, centre.y, centre.z}};
    octants[2] = {Vector3F{centre.x, this->region.get_minimum().y, centre.z}, Vector3F{this->region.get_maximum().x, centre.y, this->region.get_maximum().z}};
    octants[3] = {Vector3F{this->region.get_minimum().x, this->region.get_minimum().y, centre.z}, Vector3F{centre.x, centre.y, this->region.get_maximum().z}};
    octants[4] = {Vector3F{this->region.get_minimum().x, centre.y, this->region.get_minimum().z}, Vector3F{centre.x, this->region.get_maximum().y, centre.z}};
    octants[5] = {Vector3F{centre.x, centre.y, this->region.get_minimum().z}, Vector3F{this->region.get_maximum().x, this->region.get_maximum().y, centre.z}};
    octants[6] = {centre, this->region.get_maximum()};
    octants[7] = {Vector3F{this->region.get_minimum().x, centre.y, centre.z}, Vector3F{centre.x, this->region.get_maximum().y, this->region.get_maximum().z}};

    std::array<std::vector<Renderable*>, 8> octant_list;
    std::vector<Renderable*> delisted_objects;
    for(Renderable* object : this->enclosed_objects)
    {
        // If the enclosed object has no value, then we don't care about partitioning it anyway.
        if(!object->get_boundary().has_value())
            continue;
        AABB bound = object->get_boundary().value();
        if(bound.get_minimum() != bound.get_maximum())
        {
            // If the bound has non-negligible size:
            for(std::size_t i = 0; i < 8; i++)
            {
                if(octants[i].contains(bound)) // Might have to fully contain the AABB instead of just intersecting.
                {
                    octant_list[i].push_back(object);
                    delisted_objects.push_back(object);
                    break;
                }
            }
        }
        // Check here for other boundary types. (Or use a generic boundary if you can)
    }

    // For every object marked as de-listed, actually remove them.
    for(Renderable* to_delist : delisted_objects)
    {
        this->enclosed_objects.erase(std::remove(this->enclosed_objects.begin(), this->enclosed_objects.end(), to_delist));
    }

    for(std::size_t i = 0; i < 8; i++)
    {
        if(!octant_list[i].empty())
        {
            this->children[i] = this->create_node(octants[i], octant_list[i]);
            this->child_mask |= (1 << i);
            this->children[i]->build();
        }
    }

    this->fully_built = true;
    this->ready = true;
}

bool ScenePartitionNode::insert(Renderable* object)
{
    if(object == nullptr || !object->get_boundary().has_value()) // If our object is invalid in anyway, don't do anything with it.
        return false;
    // We already have a tree, let's insert something without having to rebuild the whole thing.
    if(this->enclosed_objects.empty()) // No structure anyway, so can trivially add it.
    {
        this->enclosed_objects.push_back(object);
        return true;
    }

    // Are the dimensions greater than the minimum dimensions?
    Vector3F dimensions = this->region.get_maximum() - this->region.get_minimum();
    if(dimensions <= tz::scene::minimum_node_size)
    {
        this->enclosed_objects.push_back(object);
        return true;
    }

    // Object won't fit into the current region, so it's not going to fit in any children. So let's try and push it upwards in the tree. If we're the root, then we're going to have to resize the whole thing.
    if(this->region.contains(object->get_boundary().value()))
    {
        if(this->parent != nullptr)
            return this->parent->insert(object);
        else
            return false;
    }

    // If we get here, then we know this region can contain the object BUT there are child nodes. Time to find out if it fits in a sub-region.
    Vector3F half_dimensions = dimensions / 2.0f;
    Vector3F centre = this->region.get_minimum() + half_dimensions;

    std::array<AABB, 8> child_octants{AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}, AABB{Vector3F{}, Vector3F{}}};
    child_octants[0] = (this->children[0] != nullptr) ? this->children[0]->region : AABB(region.get_minimum(), centre);
    child_octants[1] = (this->children[1] != nullptr) ? this->children[1]->region : AABB(Vector3F(centre.x, region.get_minimum().y, region.get_minimum().z), Vector3F(region.get_maximum().x, centre.y, centre.z));
    child_octants[2] = (this->children[2] != nullptr) ? this->children[2]->region : AABB(Vector3F(centre.x, region.get_minimum().y, centre.z), Vector3F(region.get_maximum().x, centre.y, region.get_maximum().z));
    child_octants[3] = (this->children[3] != nullptr) ? this->children[3]->region : AABB(Vector3F(region.get_minimum().x, region.get_minimum().y, centre.z), Vector3F(centre.x, centre.y, region.get_maximum().z));
    child_octants[4] = (this->children[4] != nullptr) ? this->children[4]->region : AABB(Vector3F(region.get_minimum().x, centre.y, region.get_minimum().z), Vector3F(centre.x, region.get_maximum().y, centre.z));
    child_octants[5] = (this->children[5] != nullptr) ? this->children[5]->region : AABB(Vector3F(centre.x, centre.y, region.get_minimum().z), Vector3F(region.get_maximum().x, region.get_maximum().y, centre.z));
    child_octants[6] = (this->children[6] != nullptr) ? this->children[6]->region : AABB(centre, region.get_maximum());
    child_octants[7] = (this->children[7] != nullptr) ? this->children[7]->region : AABB(Vector3F(region.get_minimum().x, centre.y, centre.z), Vector3F(centre.x, region.get_maximum().y, region.get_maximum().z));

    // Is the object completely within the root box?
    auto box = object->get_boundary().value();
    if(box.get_maximum() != box.get_minimum() && this->region.contains(box))
    {
        bool found = false;
        for(std::size_t i = 0; i < 8; i++)
        {
            // is the object fully within an octant?
            if(child_octants[i].contains(box))
            {
                if(this->children[i] != nullptr)
                    return this->children[i]->insert(object); // Make the child deal with it.
                else
                {
                    this->children[i] = this->create_node(child_octants[i], {object});
                    this->child_mask |= (1 << i);
                }
                found = true;
            }
        }

        // We couldn't fit the object in a child, so it's going in here.
        if(!found)
        {
            this->enclosed_objects.push_back(object);
            return true;
        }
    }
    // handle other boundaries here... if necessary.
    // otherwise, the object lies either outside of the enclosed box or is intersecting it. for each case, we do need to rebuild the entire tree.
    return false;
}

Scene::Scene(): objects{}, directional_lights{}, point_lights{}, objects_to_delete{}, octree{this}{}

void Scene::render(RenderPass render_pass) const
{
    using namespace tz::utility;
    Shader* render_shader = render_pass.get_render_context().object_shader;
    Shader* sprite_shader = render_pass.get_render_context().sprite_shader;
    const Camera& camera = render_pass.get_camera();
    Vector2I viewport_dimensions = {render_pass.get_window().get_width(), render_pass.get_window().get_height()};
    BoundingPyramidalFrustum camera_frustum(camera, viewport_dimensions.x / viewport_dimensions.y);
    auto render_if_visible = [&](const StaticObject& object){if(object.get_asset().valid_model()){object.render(render_pass);return;} AABB object_box = tz::physics::bound_aabb(object.get_asset()).value(); if(camera_frustum.contains(object_box * object.transform.model()) || tz::graphics::is_instanced(object.get_asset().mesh)) object.render(render_pass);};
    if(render_shader != nullptr)
    {
        for (auto &static_object : this->get_static_objects())
        {
            if (generic::contains(this->objects_to_delete, const_cast<Renderable*>(dynamic_cast<const Renderable*>(&static_object.get()))))
                continue;
            render_if_visible(static_object.get());
        }
    }
    if(sprite_shader != nullptr)
    {
        for(auto& sprite : this->get_sprites())
        {
            if(generic::contains(this->objects_to_delete, const_cast<Renderable*>(dynamic_cast<const Renderable*>(&sprite.get()))))
                continue;
            sprite.get().render(render_pass);
        }
    }
    if(render_shader == nullptr)
        return;
    for(std::size_t i = 0; i < this->directional_lights.size(); i++)
    {
        render_shader->bind();
        render_shader->set_uniform<DirectionalLight>(std::string("directional_lights[") + std::to_string(i) + "]", this->directional_lights[i]);
    }
    for(std::size_t i = 0; i < this->point_lights.size(); i++)
    {
        render_shader->bind();
        render_shader->set_uniform<PointLight>(std::string("point_lights[") + std::to_string(i) + "]", this->point_lights[i]);
    }
    render_shader->update();
}

void Scene::update(float delta_time)
{
    this->octree.update();
    for(std::reference_wrapper<DynamicObject> dynamic_ref : this->get_mutable_dynamic_objects())
        dynamic_ref.get().update(delta_time);
    for(std::reference_wrapper<DynamicSprite> dynamic_sprite_ref : this->get_mutable_dynamic_sprites())
        dynamic_sprite_ref.get().update(delta_time);
    std::vector<std::reference_wrapper<PhysicsObject>> physics_sprites;
    using namespace tz::utility;
    for(auto& object : this->get_mutable_static_objects())
    {
        std::vector<std::reference_wrapper<PhysicsObject>> physics_objects;
        if(std::find(this->objects_to_delete.begin(), this->objects_to_delete.end(), &object.get()) != this->objects_to_delete.end())
            continue;
        auto physics_component = dynamic_cast<PhysicsObject*>(&object.get());
        if(physics_component != nullptr)
        {
            // this object has a physics component, so check for it in its own node.
            const ScenePartitionNode* enclosed_node = this->octree.get_node_containing(&object.get());
            // we have the node containing this object, now we get all the PhysicsObjects in this node and handle collisions on it.
            for(const Renderable& renderable_cref : enclosed_node->get_enclosed_renderables())
            {
                Renderable* fixed = const_cast<Renderable*>(&renderable_cref);
                if(functional::is_a<Renderable, DynamicObject>(*fixed))
                {
                    DynamicObject* fixed_dyno = dynamic_cast<DynamicObject*>(fixed);
                    physics_objects.push_back(std::ref(*dynamic_cast<PhysicsObject*>(fixed_dyno)));
                }
            }
            //tz::debug::print("Scene::update(...): Handling collision of node of size ", physics_objects.size(), "...\n");
            physics_component->handle_collisions(physics_objects);
        }
    }
    for(auto& sprite : this->get_mutable_sprites())
    {
        auto physics_component = dynamic_cast<PhysicsObject*>(&sprite.get());
        if(physics_component != nullptr)
            physics_sprites.push_back(std::ref(*physics_component));
    }
    /*
    std::multimap<float, std::reference_wrapper<PhysicsObject>> physics_objects_sweeped;
    for(auto& [value, dynamic_object_ref] : this->get_mutable_dynamic_objects_sorted_by_variance_axis())
    {
        DynamicObject& dynamic_object = dynamic_object_ref.get();
        physics_objects_sweeped.emplace(value, *dynamic_cast<PhysicsObject*>(&dynamic_object));
    }
    std::multimap<float, std::reference_wrapper<PhysicsObject>> physics_sprites_sweeped;
    for(auto& [value, dynamic_sprite_ref] : this->get_mutable_dynamic_sprites_sorted_by_variance_axis())
    {
        DynamicSprite& dynamic_sprite = dynamic_sprite_ref.get();
        physics_sprites_sweeped.emplace(value, *dynamic_cast<PhysicsObject*>(&dynamic_sprite));
    }
    for(PhysicsObject& object : physics_objects)
        object.handle_collisions_sort_and_sweep(this->get_highest_variance_axis_objects(), physics_objects_sweeped);
    for(PhysicsObject& sprite_object : physics_sprites)
        sprite_object.handle_collisions_sort_and_sweep(this->get_highest_variance_axis_sprites(), physics_sprites_sweeped);
    */
    this->handle_deletions();
}

std::size_t Scene::get_number_of_static_objects() const
{
    return this->get_static_objects().size();
}

std::size_t Scene::get_number_of_sprites() const
{
    return this->get_sprites().size();
}

std::size_t Scene::get_number_of_elements() const
{
    return this->get_number_of_static_objects() + this->get_number_of_sprites();
}

std::vector<std::reference_wrapper<const StaticObject>> Scene::get_static_objects() const
{
    std::vector<std::reference_wrapper<const StaticObject>> object_crefs;
    for(const auto& renderable_ptr : this->objects)
    {
        auto static_ptr = dynamic_cast<const StaticObject*>(renderable_ptr.get());
        if(static_ptr != nullptr)
            object_crefs.push_back(std::cref(*static_ptr));
    }
    return object_crefs;
}

std::vector<std::reference_wrapper<const Sprite>> Scene::get_sprites() const
{
    std::vector<std::reference_wrapper<const Sprite>> sprite_crefs;
    for(const auto& renderable_ptr : this->objects)
    {
        auto sprite_ptr = dynamic_cast<const Sprite*>(renderable_ptr.get());
        if(sprite_ptr != nullptr)
            sprite_crefs.push_back(std::cref(*sprite_ptr));
    }
    return sprite_crefs;
}

AABB Scene::get_boundary(std::optional<std::pair<const Camera&, Vector2I>> frustum_culling) const
{
    auto is_visible = [&](const StaticObject& object) -> bool
    {
        if(!frustum_culling.has_value())
            return true;
        BoundingPyramidalFrustum camera_frustum(frustum_culling.value().first, frustum_culling.value().second.x / frustum_culling.value().second.y);
        std::optional<AABB> object_box = tz::physics::bound_aabb(object.get_asset());
        return (object_box.has_value() && camera_frustum.contains(object_box.value())) || tz::graphics::is_instanced(object.get_asset().mesh);
    };
    auto objects = this->get_static_objects();
    if(objects.size() == 0)
        return {{}, {}};
    Vector3F min = objects.front().get().transform.position, max = objects.front().get().transform.position;
    for(const StaticObject& object : objects)
    {
        auto boundary_optional = object.get_boundary();
        if(!boundary_optional.has_value() || !is_visible(object))
            continue;
        const AABB& boundary = boundary_optional.value();
        min.x = std::min(min.x, boundary.get_minimum().x);
        min.y = std::min(min.y, boundary.get_minimum().y);
        min.z = std::min(min.z, boundary.get_minimum().z);

        max.x = std::max(max.x, boundary.get_maximum().x);
        max.y = std::max(max.y, boundary.get_maximum().y);
        max.z = std::max(max.z, boundary.get_maximum().z);
    }
    return {min, max};
}


void Scene::remove_object(StaticObject& object)
{
    this->objects_to_delete.push_back(&object);
}

void Scene::remove_sprite(Sprite& sprite)
{
    this->objects_to_delete.push_back(&sprite);
}

std::optional<DirectionalLight> Scene::get_directional_light(std::size_t light_id) const
{
    try
    {
        return {this->directional_lights.at(light_id)};
    }
    catch(const std::out_of_range& range_exception)
    {
        return {};
    }
}

void Scene::set_directional_light(std::size_t light_id, DirectionalLight light)
{
    if(light_id >= this->directional_lights.size())
        this->directional_lights.resize(light_id + 1);
    this->directional_lights.at(light_id) = light;
}

void Scene::add_directional_light(DirectionalLight light)
{
    this->directional_lights.push_back(std::move(light));
}

std::optional<PointLight> Scene::get_point_light(std::size_t light_id) const
{
    try
    {
        return {this->point_lights.at(light_id)};
    }
    catch(const std::out_of_range& range_exception)
    {
        return {};
    }
}

void Scene::set_point_light(std::size_t light_id, PointLight light)
{
    if(light_id >= this->point_lights.size())
        this->point_lights.resize(light_id + 1);
    this->point_lights.at(light_id) = light;
}

void Scene::add_point_light(PointLight light)
{
    this->point_lights.push_back(std::move(light));
}

const ScenePartitionNode& Scene::get_octree_root() const
{
    return this->octree;
}

std::vector<std::reference_wrapper<const DynamicObject>> Scene::get_dynamic_objects() const
{
    return tz::utility::functional::get_subclasses<const StaticObject, const DynamicObject>(this->get_static_objects());
}

std::vector<std::reference_wrapper<StaticObject>> Scene::get_mutable_static_objects()
{
    std::vector<std::reference_wrapper<StaticObject>> object_refs;
    for(auto& renderable_ptr : this->objects)
    {
        auto static_ptr = dynamic_cast<StaticObject*>(renderable_ptr.get());
        if(static_ptr != nullptr)
            object_refs.push_back(std::ref(*static_ptr));
    }
    return object_refs;
}

std::vector<std::reference_wrapper<DynamicObject>> Scene::get_mutable_dynamic_objects()
{
    return tz::utility::functional::get_subclasses<StaticObject, DynamicObject>(this->get_mutable_static_objects());
}

std::vector<std::reference_wrapper<const DynamicSprite>> Scene::get_dynamic_sprites() const
{
    return tz::utility::functional::get_subclasses<const Sprite, const DynamicSprite>(this->get_sprites());
}

std::vector<std::reference_wrapper<Sprite>> Scene::get_mutable_sprites()
{
    std::vector<std::reference_wrapper<Sprite>> sprite_refs;
    for(auto& renderable_ptr : this->objects)
    {
        auto sprite_ptr = dynamic_cast<Sprite*>(renderable_ptr.get());
        if(sprite_ptr != nullptr)
            sprite_refs.push_back(std::ref(*sprite_ptr));
    }
    return sprite_refs;
}

std::vector<std::reference_wrapper<DynamicSprite>> Scene::get_mutable_dynamic_sprites()
{
    return tz::utility::functional::get_subclasses<Sprite, DynamicSprite>(this->get_mutable_sprites());
}

std::multimap<float, std::reference_wrapper<DynamicObject>> Scene::get_mutable_dynamic_objects_sorted_by_variance_axis()
{
    std::multimap<float, std::reference_wrapper<DynamicObject>> sorted_dyn_objects;
    using namespace tz::physics;
    Axis3D highest_variance_axis = this->get_highest_variance_axis_objects();
    for(DynamicObject& object : this->get_mutable_dynamic_objects())
    {
        if(object.get_boundary().has_value())
        {
            AABB bound = object.get_boundary().value();
            float value;
            switch(highest_variance_axis)
            {
                case Axis3D::X:
                default:
                    value = bound.get_minimum().x;
                    break;
                case Axis3D::Y:
                    value = bound.get_minimum().y;
                    break;
                case Axis3D::Z:
                    value = bound.get_minimum().z;
                    break;
            }
            sorted_dyn_objects.emplace(value, std::ref(object));
        }
    }
    return sorted_dyn_objects;
}

std::multimap<float, std::reference_wrapper<DynamicSprite>> Scene::get_mutable_dynamic_sprites_sorted_by_variance_axis()
{
    std::multimap<float, std::reference_wrapper<DynamicSprite>> sorted_dyn_sprites;
    using namespace tz::physics;
    Axis2D highest_variance_axis = this->get_highest_variance_axis_sprites();
    for(DynamicSprite& sprite : this->get_mutable_dynamic_sprites())
    {
        if(sprite.get_boundary().has_value())
        {
            AABB bound = sprite.get_boundary().value();
            float value;
            switch(highest_variance_axis)
            {
                case Axis2D::X:
                default:
                    value = bound.get_minimum().x;
                    break;
                case Axis2D::Y:
                    value = bound.get_minimum().y;
                    break;
            }
            sorted_dyn_sprites.emplace(value, std::ref(sprite));
        }
    }
    return sorted_dyn_sprites;
}

tz::physics::Axis3D Scene::get_highest_variance_axis_objects() const
{
    std::vector<float> values_x, values_y, values_z;
    for(const DynamicObject& object : this->get_dynamic_objects())
    {
        if(object.get_boundary().has_value())
        {
            AABB bound = object.get_boundary().value();
            values_x.push_back(bound.get_minimum().x);
            values_y.push_back(bound.get_minimum().y);
            values_z.push_back(bound.get_minimum().z);
        }
    }
    using namespace tz::utility;
    using namespace tz::physics;
    float sigma_x = numeric::variance(values_x), sigma_y = numeric::variance(values_y), sigma_z = numeric::variance(values_z);
    if(sigma_x >= sigma_y && sigma_x >= sigma_z)
        return Axis3D::X;
    else if(sigma_y >= sigma_x && sigma_y >= sigma_z)
        return Axis3D::Y;
    else if(sigma_z >= sigma_x && sigma_z >= sigma_y)
        return Axis3D::Z;
    else
        return Axis3D::X;
}

tz::physics::Axis2D Scene::get_highest_variance_axis_sprites() const
{
    std::vector<float> values_x, values_y;
    for(const DynamicSprite& sprite : this->get_dynamic_sprites())
    {
        if(sprite.get_boundary().has_value())
        {
            AABB bound = sprite.get_boundary().value();
            values_x.push_back(bound.get_minimum().x);
            values_y.push_back(bound.get_minimum().y);
        }
    }
    using namespace tz::utility;
    using namespace tz::physics;
    float sigma_x = numeric::variance(values_x), sigma_y = numeric::variance(values_y);
    if(sigma_x >= sigma_y)
        return Axis2D::X;
    else
        return Axis2D::Y;
}

void Scene::erase_object(Renderable* to_delete)
{
    auto renderable_iterator = std::remove_if(this->objects.begin(), this->objects.end(), [&](const auto& renderable_ptr){return renderable_ptr.get() == to_delete;});
    if(renderable_iterator != this->objects.end())
        this->objects.erase(renderable_iterator);
}

/*
void Scene::erase_object(StaticObject* to_delete)
{
    auto stack_iterator = std::remove(this->stack_objects.begin(), this->stack_objects.end(), *to_delete);
    if(stack_iterator != this->stack_objects.end())
    {
        this->stack_objects.erase(stack_iterator);
    }
    auto heap_iterator = std::remove_if(this->heap_objects.begin(), this->heap_objects.end(), [&](const auto& object_ptr){return object_ptr.get() == to_delete;});
    if(heap_iterator != this->heap_objects.end())
    {
        this->heap_objects.erase(heap_iterator);
    }
}

void Scene::erase_sprite(Sprite* to_delete)
{
    auto stack_iterator = std::remove(this->stack_sprites.begin(), this->stack_sprites.end(), *to_delete);
    if(stack_iterator != this->stack_sprites.end())
    {
        this->stack_sprites.erase(stack_iterator);
    }
    auto heap_iterator = std::remove_if(this->heap_sprites.begin(), this->heap_sprites.end(), [&](const auto& sprite_ptr){return sprite_ptr.get() == to_delete;});
    if(heap_iterator != this->heap_sprites.end())
    {
        this->heap_sprites.erase(heap_iterator);
    }
}
 */

void Scene::handle_deletions()
{
    for(Renderable* deletion : this->objects_to_delete)
        this->erase_object(deletion);
    this->objects_to_delete.clear();
    /*
    for(StaticObject* deletion : this->objects_to_delete)
        this->erase_object(deletion);
    this->objects_to_delete.clear();
    for(Sprite* deletion : this->sprites_to_delete)
        this->erase_sprite(deletion);
    this->sprites_to_delete.clear();
     */
}