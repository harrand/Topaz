#include "core/scene.hpp"

ScenePartitionNode::ScenePartitionNode(Scene* scene, AABB region, std::vector<const Renderable*> enclosed_objects): scene(scene), parent(nullptr), region(region), children({nullptr}), pending_insertion(), enclosed_objects(enclosed_objects), child_mask(0x00), fully_built(false), ready(false){}

ScenePartitionNode::ScenePartitionNode(ScenePartitionNode&& move): scene(move.scene), parent(nullptr), region(move.region), children(std::move(move.children)), pending_insertion(std::move(move.pending_insertion)), enclosed_objects(std::move(move.enclosed_objects)), child_mask(move.child_mask), fully_built(move.fully_built), ready(move.ready)
{
	topaz_assert(move.parent == nullptr, "ScenePartitionNode(ScenePartitionNode&&): Move constructor invoked on a non root-node. move.parent == ", move.parent, " but should be 0x00 (nullptr).");
	for(auto& child_ptr : this->children)
		if(child_ptr != nullptr)
			child_ptr->parent = this;
}

ScenePartitionNode& ScenePartitionNode::operator=(ScenePartitionNode&& rhs)
{
	topaz_assert(rhs.parent == nullptr && this->parent == nullptr, "ScenePartitionNode::operator=(ScenePartitionNode&&): Move assignment operator invoked, but either lhs or rhs is not a root node. Both sides MUST be a root node.");
	this->scene = rhs.scene;
	this->region = rhs.region;
	this->children = std::move(rhs.children);
	for(auto& child_ptr : this->children)
		if(child_ptr != nullptr)
			child_ptr->parent = this;
	this->pending_insertion = std::move(rhs.pending_insertion);
	this->enclosed_objects = std::move(rhs.enclosed_objects);
	this->child_mask = rhs.child_mask;
	this->fully_built = rhs.fully_built;
	this->ready = rhs.ready;
	return *this;
}

void ScenePartitionNode::enqueue_object(const Renderable *object)
{
	this->pending_insertion.push(object);
}

const std::vector<std::reference_wrapper<const Renderable>> ScenePartitionNode::get_enclosed_renderables() const
{
	std::vector<std::reference_wrapper<const Renderable>> renderable_crefs;
	for(auto* renderable_ptr : this->enclosed_objects)
		renderable_crefs.push_back(std::cref(*renderable_ptr));
	for(const auto& child_ptr : this->children)
	{
		if (child_ptr != nullptr)
		{
			for(const Renderable& renderable : child_ptr->get_enclosed_renderables())
				renderable_crefs.push_back(std::cref(renderable));
		}
	}
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

const ScenePartitionNode* ScenePartitionNode::get_node_containing(const Renderable* object) const
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

std::unique_ptr<ScenePartitionNode> ScenePartitionNode::create_node(AABB region, std::vector<const Renderable*> enclosed_objects)
{
	if(enclosed_objects.empty())
		return nullptr;
	std::unique_ptr<ScenePartitionNode> child = std::make_unique<ScenePartitionNode>(this->scene, region, enclosed_objects);
	child->parent = this;
	return child;
}

void ScenePartitionNode::find_enclosing_cube()
{
	/*
	auto min = scene->get_boundary().get_minimum() * 2.0f;
	auto max = scene->get_boundary().get_maximum() * 2.0f;
	this->region = {min, max};
	*/
	this->region = scene->get_boundary();
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
		tz::debug::print("ScenePartitionNode::build(): Invoked, but the dimensions of the region were empty, so giving it a default size...\ni");
	}

	// Are our dimensions smaller than the minimum allowed size?
	using namespace tz::consts::core::scene;
	if(dimensions <= Vector3F{octree::minimum_node_size_x, octree::minimum_node_size_y, octree::minimum_node_size_z})
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

	std::array<std::vector<const Renderable*>, 8> octant_list;
	std::vector<const Renderable*> delisted_objects;
	for(const Renderable* object : this->enclosed_objects)
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
	for(const Renderable* to_delist : delisted_objects)
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

bool ScenePartitionNode::insert(const Renderable* object)
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
	using namespace tz::consts::core::scene;
	if(dimensions <= Vector3F{octree::minimum_node_size_x, octree::minimum_node_size_y, octree::minimum_node_size_z})
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

Scene::Scene(ScenePartitionType type): object_buffer{0}, objects{}, inheritance_map{}, directional_lights{}, point_lights{}, objects_to_delete{}, octree{std::nullopt}
{
	switch(type)
	{
		case ScenePartitionType::OCTREE:
			this->octree = {ScenePartitionNode{this}};
			break;
		default:
		case ScenePartitionType::NONE:
			this->octree = {std::nullopt};
			break;
	}
}

Scene::Scene(const Scene& copy): object_buffer{0}, objects{}, inheritance_map{}, directional_lights{copy.directional_lights}, point_lights{copy.point_lights}, objects_to_delete{}, octree{this}
{
	for(auto& renderable_ptr : copy.objects)
	{
		//this->objects.push_back(renderable_ptr->unique_clone());
		//Renderable* deep_copied = this->objects.back().get();
        Renderable* deep_copied = this->object_buffer.take_object(renderable_ptr->unique_clone());
        this->objects.push_back(deep_copied);
		if(!tz::utility::functional::is_a<Renderable, Sprite>(*deep_copied) && this->octree.has_value())
			this->octree.value().enqueue_object(deep_copied);
		this->inheritance_map.insert({typeid(*deep_copied), deep_copied});
		std::cout << "Scene::Scene(const Scene& copy): Copied scene element of type " << typeid(*deep_copied).name() << ".\n";
	}
}

Scene::Scene(Scene&& move): object_buffer(std::move(move.object_buffer)), objects(std::move(move.objects)), inheritance_map(std::move(move.inheritance_map)), directional_lights(std::move(move.directional_lights)), point_lights(std::move(move.point_lights)), objects_to_delete{}, octree(std::move(move.octree))
{
	// Delete all pending deletions now.
	for(Renderable* to_delete : move.objects_to_delete)
		this->erase_object(to_delete);
}

Scene& Scene::operator=(Scene rhs)
{
	Scene::swap(*this, rhs);
	return *this;
}

Scene& Scene::operator=(Scene&& rhs)
{
    this->object_buffer = std::move(rhs.object_buffer);
	this->objects = std::move(rhs.objects);
	this->inheritance_map = std::move(rhs.inheritance_map);
	this->directional_lights = std::move(rhs.directional_lights);
	this->point_lights = std::move(rhs.point_lights);
	this->objects_to_delete = std::move(rhs.objects_to_delete);
	this->octree = std::move(rhs.octree);
	return *this;
}

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
		// MDI Disabled
		for (auto &static_object : this->get_static_objects())
		{
			if (generic::contains(this->objects_to_delete, const_cast<Renderable*>(dynamic_cast<const Renderable*>(&static_object.get()))))
				continue;
			render_if_visible(static_object.get());
		}

		/*
		// MDI Enabled.
		this->object_buffer.render(render_pass);
		 */
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
	// Update the octree, ensuring that any enqueued objects are inserted properly.
	if(this->octree.has_value())
		this->octree.value().update();
	// Invoke update functions on all dynamic objects and sprites.
	for(auto* renderable : this->objects)
	{
		PhysicsObject* physics_component = dynamic_cast<PhysicsObject*>(renderable);
		if(physics_component != nullptr)
			physics_component->update(delta_time);
	}
	std::vector<std::reference_wrapper<PhysicsObject>> physics_sprites;
	using namespace tz::utility;
	// For each static object, get all those who are also PhysicsObjects.
	// from those PhysicsObjects, get the octree node containing this object, and handle collisions with all of those.
	for(auto* object : this->get_renderables_by_type<StaticObject>())
	{
		std::vector<std::reference_wrapper<PhysicsObject>> physics_objects;
		if(std::find(this->objects_to_delete.begin(), this->objects_to_delete.end(), object) != this->objects_to_delete.end())
			continue;
		auto physics_component = dynamic_cast<PhysicsObject*>(object);
		if(physics_component != nullptr)
		{
			if(this->octree.has_value())
			{
				// this object has a physics component, so check for it in its own node.
				const ScenePartitionNode *enclosed_node = this->octree.value().get_node_containing(object);
				// we have the node containing this object, now we get all the PhysicsObjects in this node and handle collisions on it.
				for (const Renderable &renderable_cref : enclosed_node->get_enclosed_renderables())
				{
					auto *fixed = const_cast<Renderable *>(&renderable_cref);
					if (functional::is_a<Renderable, DynamicObject>(*fixed))
					{
						auto *fixed_dyno = static_cast<DynamicObject*>(fixed);
						physics_objects.push_back(std::ref(*static_cast<PhysicsObject*>(fixed_dyno)));
					}
				}
			}
			else
			{
				for(const auto dyn_object_ptr : this->get_renderables_by_type<DynamicObject>())
					physics_objects.push_back(std::ref(*static_cast<PhysicsObject*>(dyn_object_ptr)));
			}
			//tz::debug::print("Scene::update(...): Handling collision of node of size ", physics_objects.size(), "...\n");
			physics_component->handle_collisions(physics_objects);
		}
	}
	for(auto* sprite : this->get_renderables_by_type<Sprite>())
	{
		auto physics_component = dynamic_cast<PhysicsObject*>(sprite);
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
		auto static_ptr = dynamic_cast<const StaticObject*>(renderable_ptr);
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
		auto sprite_ptr = dynamic_cast<const Sprite*>(renderable_ptr);
		if(sprite_ptr != nullptr)
			sprite_crefs.push_back(std::cref(*sprite_ptr));
	}
	return sprite_crefs;
}

AABB Scene::get_boundary(std::optional<std::pair<const Camera&, Vector2I>> frustum_culling) const
{
	// Perform frustum culling on boundary.
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
		// Compute the minimum and maximum for all StaticObjects, resulting in the resultant AABB.
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

bool Scene::contains_octree() const
{
	return this->octree.has_value();
}

const ScenePartitionNode* Scene::get_octree_root() const
{
	if(this->octree.has_value())
		return &this->octree.value();
	else
		return nullptr;
}

std::unordered_set<const Renderable*> Scene::raycast(Vector2I screen_position, RenderPass render_pass) const
{
	std::unordered_set<const Renderable*> collided;
	const Camera& camera = render_pass.get_camera();
	// get the screen position, and convert it into a ray-direction in world-space.
	Vector3F ray_normalised_device_coordinates{(2.0f * screen_position.x) / render_pass.get_window().get_width() - 1.0f, 1.0f - (2.0f * screen_position.y) / render_pass.get_window().get_height(), 1.0f};
	Vector4F ray_clip_space{ray_normalised_device_coordinates.xy(), {-1.0f, 1.0f}};
	Vector4F ray_camera_space = camera.projection(render_pass.get_window().get_width(), render_pass.get_window().get_height()).inverse() * ray_clip_space;
	ray_camera_space = {ray_camera_space.xy(), {-1.0f, 0.0f}};
	Vector3F ray_world_space = (camera.view().inverse() * ray_camera_space).xyz().normalised();
	// Construct a BoundingLine (the ray itself) from the camera position and with the computed direction
	BoundingLine ray{camera.position, ray_world_space};
	// This ignores the octree entirely and does it naively.
	if(!this->octree.has_value())
	{
		for (const auto &object_ptr : this->objects)
			if (object_ptr->get_boundary().has_value() && object_ptr->get_boundary().value().intersects(ray))
				collided.insert(object_ptr);
		return collided;
	}
	const auto* node = &this->octree.value();
	bool early_exit = false;
	// Get the smallest node whose region intersects this ray.
	// The resultant set shall contain all objects within this smallest region that actually collide with the ray.
	while(node->region.intersects(ray) && !early_exit)
	{
		// Check each child manually
		for(std::size_t i = 0; i < 8; i++)
		{
			if(node->get_child(i) == nullptr)
			{
				if(i == 7)
					early_exit = true;
				continue;
			}
			if(node->get_child(i)->region.intersects(ray))
			{
				node = node->get_child(i);
				break;
			}
			// If no child nodes intersect the ray, then we keep the current node.
			early_exit = true;
			break;
		}
	}
	// Assume by this point, we've found the smallest node which this ray intersects. Now we check collision normally.
	for(const Renderable& object : node->get_enclosed_renderables())
		if(object.get_boundary().has_value() && object.get_boundary().value().intersects(ray))
			collided.insert(&object);//return &object;
	return collided;
}

const Renderable* Scene::get_renderable_by_id(std::size_t index) const
{
	const Renderable* ret;
	try
	{
		ret = this->objects.at(index);
	}
	catch(...)
	{
		ret = nullptr;
	}
	return ret;
}

Renderable* Scene::get_renderable_by_id(std::size_t index)
{
	Renderable* ret;
	try
	{
		ret = this->objects.at(index);
	}
	catch(...)
	{
		ret = nullptr;
	}
	return ret;
}

void Scene::swap(Scene& lhs, Scene& rhs)
{
	std::swap(lhs.objects, rhs.objects);
	std::swap(lhs.inheritance_map, rhs.inheritance_map);
	std::swap(lhs.directional_lights, rhs.directional_lights);
	std::swap(lhs.point_lights, rhs.point_lights);
	std::swap(lhs.objects_to_delete, rhs.objects_to_delete);
	std::swap(lhs.octree, rhs.octree);
}

void Scene::erase_object(Renderable* to_delete)
{
	auto renderable_iterator = std::remove_if(this->objects.begin(), this->objects.end(), [&](const auto& renderable_ptr){return renderable_ptr == to_delete;});
	if(renderable_iterator != this->objects.end())
		this->objects.erase(renderable_iterator);
}

void Scene::handle_deletions()
{
	for(Renderable* deletion : this->objects_to_delete)
		this->erase_object(deletion);
	this->objects_to_delete.clear();
}