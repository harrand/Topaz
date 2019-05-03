#ifndef TOPAZ_SCENE_HPP
#define TOPAZ_SCENE_HPP
#include "physics/dynamic_object.hpp"
#include "physics/dynamic_sprite.hpp"
#include "physics/physics.hpp"
#include "graphics/light.hpp"
#include "data/tree.hpp"
#include <map>
#include <queue>
#include <typeindex>

class Scene;

enum class ScenePartitionType
{
	OCTREE,
	NONE
};

/**
 * Node of an Octree used in the Scene space partition.
 */
class ScenePartitionNode : public Octree<ScenePartitionNode>// : public Boundary        Not until we can test all intersection types.
{
public:
	/**
	 * Construct a Scene octree node based upon a parent scene, a region, and all objects enclosing it.
	 * @param scene - Scene containing this octree node
	 * @param region - Region of this specific node
	 * @param enclosed_objects - All renderable objects within this node
	 */
	ScenePartitionNode(Scene* scene, AABB region = {{}, {}}, std::vector<const Renderable*> enclosed_objects = {});
	ScenePartitionNode(const ScenePartitionNode& copy) = delete;
	/**
	 * Move constructor is usable for root nodes only.
	 * If the parent of this node MUST be nullptr, or an assertation will fail.
	 * @param move - Node to move from. This node must be a root node
	 */
	ScenePartitionNode(ScenePartitionNode&& move);
	ScenePartitionNode& operator=(ScenePartitionNode&& rhs);
	/**
	 * Queue an object into the Octree to be added to the tree during the next ScenePartitionNode::update() invocation.
	 * This will be invoked automatically by the parent Scene::update(...).
	 * @param object - Object to insert into the tree
	 */
	void enqueue_object(const Renderable* object);
	/**
	 * Retrieve a container of all renderable objects contained within this specific tree node
	 * @return - List of all renderables in this node or children
	 */
	const std::vector<std::reference_wrapper<const Renderable>> get_enclosed_renderables() const;
	/**
	 * Construct a cluster bounding all physical renderables in this specific tree node
	 * @return - Cluster of all objects in this node
	 */
	BoundaryCluster bound_objects() const;
	/**
	 * Retrieve the region binding this tree node
	 * @return - Region comprising this node
	 */
	const AABB& get_region() const;
	/**
	 * Retrieve a container of all direct children of this node.
	 * @return - Children of this node
	 */
	std::vector<ScenePartitionNode*> get_children() const;
	/**
	 * Query as to whether this node contains any children.
	 * If it has no children, it is a leaf-node.
	 * @return - True if node contains at least one child. Otherwise false
	 */
	bool has_children() const;
	/**
	 * Retrieve the node containing the given object, if one exists.
	 * Note that this checks the current node and all its children, it does NOT check the parent node(s).
	 * @param object - Object whose node to retrieve
	 * @return - Pointer to constant node containing the given renderable if it exists. Otherwise nullptr
	 */
	const ScenePartitionNode* get_node_containing(const Renderable* object) const;
	friend class Scene;
private:
	/**
	 * Construct a node regarding the same scene as this node.
	 * @param region - Region bounding the node
	 * @param enclosed_objects - All objects which the node encloses
	 * @return - Owning pointer to the created node
	 */
	std::unique_ptr<ScenePartitionNode> create_node(AABB region, std::vector<const Renderable*> enclosed_objects);
	/**
	 * Force this node's region to be equal to double the size of the parent scene's bounding region (maximum size)
	 */
	void find_enclosing_cube();
	/**
	 * Update the tree, causing all queued insertions to take effect.
	 */
	void update();
	/**
	 * Build the tree from scratch.
	 */
	void build();
	/**
	 * Attempt to insert a given object into the tree without attempting to re-build the whole thing.
	 * @param object - Object to insert into the tree
	 * @return - True if the insertion failed. Otherwise false. If false, then a re-build is required to insert the object
	 */
	bool insert(const Renderable* object);
	/// Parent scene.
	Scene* scene;
	/// Parent node (if this is the root node, this is nullptr).
	ScenePartitionNode* parent;
	/// Region bounding this node.
	AABB region;
	/// Container of all children. Any can be nullptr.
	std::array<std::unique_ptr<ScenePartitionNode>, 8> children;
	/// Queue of all renderables which await insertion. This is cleared every update(...) invocation.
	std::queue<const Renderable*> pending_insertion;
	/// Container of all objects enclosed within this node.
	std::vector<const Renderable*> enclosed_objects;
public:
	/// Mask containing information about which child is currently being used.
	std::uint8_t child_mask;
	/// Stores if the tree fully resembles an octree structure.
	bool fully_built;
	/// Stores if the tree has no more objects waiting to be inserted before it is complete.
	bool ready;
};

template<typename T>
class SceneSection;

/**
 * Container of all renderable objects in a given scene. Provides support for StaticObjects and DynamicObjects (3D) and Sprites and DynamicSprites (2D).
 */
class Scene
{
public:
	/**
	 * Initialise an empty scene.
	 */
	Scene(ScenePartitionType type = ScenePartitionType::OCTREE);
	Scene(const Scene& copy);
	Scene(Scene&& move);
	Scene& operator=(Scene rhs);
	Scene& operator=(Scene&& rhs);
	/**
	 * Render the scene into the currently-bound FrameBuffer.
	 * @param render_shader - The Shader with which to render all of the 3D objects in this Scene
	 * @param sprite_shader - The Shader with which to render all of the 2D objects in this Scene
	 * @param camera - The Camera whose perspective should be used in rendering the Scene
	 * @param viewport_dimensions - The dimensions (width and height) of the currently-bound FrameBuffer's viewport
	 */
	virtual void render(RenderPass render_pass) const;
	/**
	 * Invoke all tick-based (Physics) updates for all objects in the Scene.
	 * @param delta_time - The time taken, in seconds, since the last update invocation
	 */
	virtual void update(float delta_time);
	/**
	 * Get the number of 3D objects in the Scene. This includes StaticObjects, DynamicObjects and any utilised subclass of either of these.
	 * @return - The number of 3D objects in the Scene
	 */
	std::size_t get_number_of_static_objects() const;
	/**
	 * Get the number of 2D sprites in the Scene. This includes Sprites, DynamicSprites, and any utilised subclass of either of these.
	 * @return - The number of 2D sprites in the Scene
	 */
	std::size_t get_number_of_sprites() const;
	/**
	 * Get the total number of elements in the Scene, including 2D and 3D objects.
	 * @return - Total number of objects and sprites in the Scene
	 */
	std::size_t get_number_of_elements() const;
	/**
	 * Obtain a container of constant-references to all StaticObjects, DynamicObjects and any utilised subclasses of either of these in the Scene.
	 * @return - Container of constant-references to all 3D objects in the Scene
	 */
	std::vector<std::reference_wrapper<const StaticObject>> get_static_objects() const;
	/**
	 * Obtain a container of constant-references to all Sprites, DynamicSprites and any utilised subclasses of either of these in the Scene.
	 * @return - Container of constant-references to all 2D objects in the Scene
	 */
	std::vector<std::reference_wrapper<const Sprite>> get_sprites() const;
	/**
	 * Obtain an AABB engulfing all 3D objects in the Scene.
	 * @param frustum_culling_camera - Optional camera to frustum-cull the scene before bounding it.
	 * @return - AABB containing the whole Scene
	 */
	AABB get_boundary(std::optional<std::pair<const Camera&, Vector2I>> frustum_culling = std::nullopt) const;
	/**
	 * Given any element type, construct such an element in-place into this Scene.
	 * Note: This will always invoke a heap-allocation.
	 * @tparam Element - Type of element to emplace, including StaticObject, DynamicObject, Sprite, DynamicSprite and all subclasses of these classes
	 * @tparam Args - Argument types used to construct the given element
	 * @param args - Argument values used to construct the given element
	 * @return - A reference to the constructed element
	 */
	template<class Element, typename... Args>
	Element& emplace(Args&&... args);
	/**
	 * Construct a StaticObject in-place into this Scene.
	 * Note: This will always invoke a heap-allocation.
	 * Note: This is equivalent to 'Scene::emplace<StaticObject>(args...)'.
	 * @tparam Args - Argument types used to construct the given StaticObject
	 * @param args - Argument values used to construct the given StaticObject
	 * @return - A reference to the constructed StaticObject
	 */
	template<typename... Args>
	StaticObject& emplace_object(Args&&... args);
	/**
	 * Given a reference to an existing StaticObject, erase this object from the Scene in the next Scene::update() invocation.
	 * Note: This method supports polymorphic StaticObjects, including DynamicObjects.
	 * Note: The behaviour of this method will vary depending on whether the object was stack-allocated or heap-allocated:
	 *  - If stack-allocated, the first object equal to the argument shall be deleted (Shallowly-equated).
	 *  - If heap-allocated, the object matching the exact memory address to the parameter shall be deleted (Deeply-equated).
	 *  - If there is no matching object in the Scene, nothing happens.
	 * @param object - The object to delete from this Scene.
	 */
	void remove_object(StaticObject& object);
	/**
	 * Construct a Sprite in-place into this Scene.
	 * Note: This will always invoke a heap-allocation.
	 * Note: This is equivalent to 'Scene::emplace<Sprite>(args...)'.
	 * @tparam Args - Argument types used to construct the given Sprite
	 * @param args - Argument values used to construct the given Sprite
	 * @return - A reference to the constructed Sprite
	 */
	template<typename... Args>
	Sprite& emplace_sprite(Args&&... args);
	/**
	 * Given a reference to an existing Sprite, erase this object from the Scene in the next Scene::update() invocation.
	 * Note: This method supports polymorphic Sprites, including DynamicSprites.
	 * Note: The behaviour of this method will vary depending on whether the object was stack-allocated or heap-allocated:
	 *  - If stack-allocated, the first object equal to the argument shall be deleted (Shallowly-equated).
	 *  - If heap-allocated, the object matching the exact memory address to the parameter shall be deleted (Deeply-equated).
	 *  - If there is no matching object in the Scene, nothing happens.
	 * @param sprite - The sprite to delete from this Scene.
	 */
	void remove_sprite(Sprite& sprite);
	/**
	 * Get the DirectionalLight with the given ID.
	 * @param light_id - The ID of the DirectionalLight to retrieve
	 * @return - The DirectionalLight corresponding to the given ID. If no DirectionalLight exists with the ID, a null-optional is returned
	 */
	std::optional<DirectionalLight> get_directional_light(std::size_t light_id) const;
	/**
	 * Set the DirectionalLight value with the given ID.
	 * Note: If the ID is greater than the number of DirectionalLights, then the underlying container's size will expand to the given ID.
	 *  - For example, if there are 2 DirectionalLights, and Scene::set_directional_light(1000, some_light), then 997 DirectionalLights shall be default-constructed, and one copy of the given DirectionalLight is copied into ID 1000.
	 *  - If the ID of the DirectionalLight is unneeded, it is recommend to use Scene::add_directional_light(DirectionalLight) instead.
	 * @param light_id - The ID of the DirectionalLight to assign
	 * @param light - The DirectionalLight value to assign to the given ID
	 */
	void set_directional_light(std::size_t light_id, DirectionalLight light);
	/**
	 * Add a given DirectionalLight to the Scene.
	 * @param light - The DirectionalLight to add to the Scene.
	 */
	void add_directional_light(DirectionalLight light);
	/**
	 * Get the pointLight with the given ID.
	 * @param light_id - The ID of the PointLight to retrieve
	 * @return - The PointLight corresponding to the given ID. If no PointLight exists with the ID, a null-optional is returned
	 */
	std::optional<PointLight> get_point_light(std::size_t light_id) const;
	/**
	 * Set the pointLight value with the given ID.
	 * Note: If the ID is greater than the number of PointLights, then the underlying container's size will expand to the given ID.
	 *  - For example, if there are 2 PointLights, and Scene::set_point_light(1000, some_light), then 997 PointLights shall be default-constructed, and one copy of the given PointLight is copied into ID 1000.
	 *  - If the ID of the PointLight is unneeded, it is recommended to use Scene::add_point_light(PointLight) instead.
	 * @param light_id - The ID of the PointLight to assign
	 * @param light - The PointLight value to assign to the given ID
	 */
	void set_point_light(std::size_t light_id, PointLight light);
	/**
	 * Add a given PointLight to the Scene.
	 * @param light - The PointLight to add to the Scene.
	 */
	void add_point_light(PointLight light);
	/**
	 * Query as to whether this scene uses an octree for its space partitioning.
	 * @return - True if an octree is used. Otherwise false
	 */
	bool contains_octree() const;
	/**
	 * Get the root node in the octree partitioning the renderables in the scene, if it exists.
	 * @return - Pointer to octree node if it exists. Otherwise nullptr
	 */
	const ScenePartitionNode* get_octree_root() const;
	/**
	 * Simulate a raycast from the given point on the screen.
	 * @param screen_position - Position on the screen to emit a ray from
	 * @return - Set of pointers to collided objects, if any exist.
	 */
	std::unordered_set<const Renderable*> raycast(Vector2I screen_position, RenderPass render_pass) const;
	/**
	 * Given an index into the Renderable vector, return the Renderable at that index, if one exists.
	 * @param index - Index of the Renderable to return.
	 * @return - Pointer to the desired Renderable if it exists. Otherwise nullptr
	 */
	const Renderable* get_renderable_by_id(std::size_t index) const;
	/**
	 * Given an index into the Renderable vector, return the Renderable at that index, if one exists.
	 * @param index - Index of the Renderable to return.
	 * @return - Pointer to the desired Renderable if it exists. Otherwise nullptr
	 */
	Renderable* get_renderable_by_id(std::size_t index);
	/**
	 * Retrieve a section of the scene which can be iterated through. The section contains all elements of the scene which are both sub-type of Renderable and is equal to the given template parameter. Note: This does not include sub-types of the parameter.
	 * Note: This has average constant time complexity.
	 * Note: If you want a given type and all its sub-types, you must go through all Renderables and use tz::functional::is_a<Renderable, RenderableType>(renderable_ptr) instead, which would have average linear time complexity
	 * @tparam RenderableType - Type of the Renderable sub-type to search for
	 * @return - Iterable section containing all scene renderables of type RenderableType
	 */
	template<class RenderableType>
	SceneSection<RenderableType> get_renderables_by_type();
	/**
	 * Retrieve a section of the scene which can be iterated through. The section contains all elements of the scene which are both sub-type of Renderable and is equal to the given template parameter. Note: This does not include sub-types of the parameter.
	 * Note: This has average constant time complexity.
	 * Note: If you want a given type and all its sub-types, you must go through all Renderables and use tz::functional::is_a<Renderable, RenderableType>(renderable_ptr) instead, which would have average linear time complexity
	 * @tparam RenderableType - Type of the Renderable sub-type to search for
	 * @return - Iterable section containing all scene renderables of type RenderableType
	 */
	template<class RenderableType>
	SceneSection<const RenderableType> get_renderables_by_type() const;
	/**
	 * Retrieve the number of renderables in the scene exactly matching the given type.
	 * Note: This will not include sub-types of the given type.
	 * @tparam RenderableType - Type of renderable to retrieve the quantity of
	 * @return - Number of Renderables in the scene whose type exactly matches RenderableType (such as StaticObject)
	 */
	template<class RenderableType = Renderable>
	std::size_t get_number_of() const;
	template<typename T>
	friend class SceneSection;
protected:
	static void swap(Scene& lhs, Scene& rhs);
	/**
	 * Protected.
	 * Erase the given Renderable from the Scene instantly.
	 * Note: This retains the difference in behaviour mentioned in Scene::remove_object(Renderable&).
	 * @param to_delete - The Renderable to erase from the Scene
	 */
	void erase_object(Renderable* to_delete);
	/**
	 * Protected.
	 * Handle all of the queued deletion requests made by Scene::remove_object(StaticObject&) and Scene::remove_sprite(Sprite&).
	 * Note: If you are using Scene::erase_object(Renderable*) instead, then this method is unneeded.
	 */
	void handle_deletions();

	/// Container of all objects in the scene which can be rendered.
	std::vector<std::unique_ptr<Renderable>> objects;
	/// Stores Renderable* referring to all possible sub-types of Renderable. E.g all StaticObjects will be in the same range, without having to do it in O(n) time checking through every renderable in the scene.
	std::unordered_multimap<std::type_index, Renderable*> inheritance_map;
	/// Container of all DirectionalLights in the Scene.
	std::vector<DirectionalLight> directional_lights;
	/// Container of all PointLights in the Scene.
	std::vector<PointLight> point_lights;
	/// Container of all objects that have been requested for deletion in the next Scene::update() invocation.
	std::vector<Renderable*> objects_to_delete;
	/// Octree for nodes.
	std::optional<ScenePartitionNode> octree;
};

template<class RenderableType>
class SceneSection
{
public:
	using IteratorType = typename decltype(std::declval<Scene>().inheritance_map)::iterator;
	using ConstIteratorType = typename decltype(std::declval<Scene>().inheritance_map)::const_iterator;
	using Range = std::pair<IteratorType, IteratorType>;
	using ConstRnage = std::pair<ConstIteratorType, ConstIteratorType>;
	using ValueType = RenderableType*;
	using ConstValueType = const RenderableType*;
	class iterator : public std::iterator<std::forward_iterator_tag, ValueType>
	{
	public:
		iterator(IteratorType iterator);
		iterator& operator++();
		ValueType operator*();
		bool operator!=(const iterator& rhs) const;
	private:
		IteratorType iter;
	};
	class const_iterator : public std::iterator<std::forward_iterator_tag, ConstValueType>
	{
	public:
		const_iterator(ConstIteratorType iterator);
		const_iterator& operator++();
		ConstValueType operator*() const;
		bool operator!=(const const_iterator& rhs) const;
	private:
		ConstIteratorType iter;
	};
	SceneSection(IteratorType begin, IteratorType end);
	std::size_t size() const;
	iterator begin();
	const_iterator cbegin() const;
	iterator end();
	const_iterator cend() const;
private:
	Range range;
};

#include "scene.inl"
#endif //TOPAZ_SCENE_HPP
