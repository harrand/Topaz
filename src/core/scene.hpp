#ifndef TOPAZ_SCENE_HPP
#define TOPAZ_SCENE_HPP
#include "physics/dynamic_object.hpp"
#include "physics/dynamic_sprite.hpp"
#include "physics/physics.hpp"
#include <map>

/**
 * Container of all renderable objects in a given scene. Provides support for StaticObjects and DynamicObjects (3D) and Sprites and DynamicSprites (2D).
 */
class Scene
{
public:
    /**
     * Given a container of StaticObjects for the stack and the heap, initialise this scene with such objects.
     * @param stack_objects - Container of StaticObjects intended for the stack (Fast but non-polymorphic)
     * @param heap_objects - Container of StaticObjects intended for the heap (Slower, but polymorphic)
     */
    Scene(const std::initializer_list<StaticObject>& stack_objects = {}, std::vector<std::unique_ptr<StaticObject>> heap_objects = {});

    /**
     * Render the scene into the currently-bound FrameBuffer.
     * @param render_shader - The Shader with which to render all of the 3D objects in this Scene
     * @param sprite_shader - The Shader with which to render all of the 2D objects in this Scene
     * @param camera - The Camera whose perspective should be used in rendering the Scene
     * @param viewport_dimensions - The dimensions (width and height) of the currently-bound FrameBuffer's viewport
     */
    virtual void render(Shader* render_shader, Shader* sprite_shader, const Camera& camera, const Vector2I& viewport_dimensions) const;
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
     * Given a StaticObject, add a copy of it into the Scene.
     * Note: This will always invoke a stack-allocation.
     * @param scene_object - The StaticObject to add to the Scene
     */
    void add_object(StaticObject scene_object);
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
     * Given a Sprite, add a copy of it into the Scene.
     * Note: This will always invoke a stack-allocation.
     * @param sprite - The Sprite to add to the Scene
     */
    void add_sprite(Sprite sprite);
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
protected:
    /**
     * Protected.
     * Obtain a container of constant-references to all DynamicObjects and any utilised subclasses of this in the Scene.
     * @return - Container of constant-references to all 3D DynamicObjects in the Scene
     */
    std::vector<std::reference_wrapper<const DynamicObject>> get_dynamic_objects() const;
    /**
     * Protected.
     * Obtain a container of non-constant-references to all StaticObjects, DynamicObjects and any utilised subclasses of either of these in the Scene.
     * @return - Container of non-constant-references to all 3D objects in the Scene
     */
    std::vector<std::reference_wrapper<StaticObject>> get_mutable_static_objects();
    /**
     * Protected.
     * Obtain a container of non-constant-references to all DynamicObjects and any utilised subclasses of this in the Scene.
     * @return - Container of non-constant-references to all 3D DynamicObjects in the Scene
     */
    std::vector<std::reference_wrapper<DynamicObject>> get_mutable_dynamic_objects();
    /**
     * Protected.
     * Obtain a container of constant-references to all DynamicSprites and any utilised subclasses of this in the Scene.
     * @return - Container of constant-references to all 2D DynamicSprites in the Scene
     */
    std::vector<std::reference_wrapper<const DynamicSprite>> get_dynamic_sprites() const;
    /**
     * Protected.
     * Obtain a container of non-constant-references to all Sprites, DynamicObjects and any utilised subclasses of either of these in the Scene.
     * @return - Container of non-constant-references to all 2D objects in the Scene
     */
    std::vector<std::reference_wrapper<Sprite>> get_mutable_sprites();
    /**
     * Protected.
     * Obtain a container of non-constant-references to all DynamicSprites and any utilised subclasses of this in the Scene.
     * @return - Container of non-constant-references to all 2D DynamicSprites in the Scene
     */
    std::vector<std::reference_wrapper<DynamicSprite>> get_mutable_dynamic_sprites();
    /**
     * Protected.
     * Obtain a container of non-constant-references to all DynamicObjects and any utilised subclasses of this in the Scene.
     * Note: These DynamicObjects are sorted in order relative to the most variant spatial-axis in the Scene.
     * - To retrieve which axis that is, Scene::get_highest_variance_axis_objects() const is available.
     * @return - Container of non-constant-references to all 2D DynamicObjects in the Scene
     */
    std::multimap<float, std::reference_wrapper<DynamicObject>> get_mutable_dynamic_objects_sorted_by_variance_axis();
    /**
     * Protected.
     * Obtain a container of non-constant-references to all DynamicSprites and any utilised subclasses of this in the Scene.
     * Note: These DynamicSprites are sorted in order relative to the most variant spatial-axis in the Scene.
     * - To retrieve which axis that is, Scene::get_highest_variance_axis_sprites() const is available.
     * @return - Container of non-constant-references to all 2D DynamicSprites in the Scene
     */
    std::multimap<float, std::reference_wrapper<DynamicSprite>> get_mutable_dynamic_sprites_sorted_by_variance_axis();
    /**
     * Protected.
     * Retrieve the spatial-axis which has the largest range for all 3D objects in the Scene.
     * @return - The most variant spatial-axis
     */
    tz::physics::Axis3D get_highest_variance_axis_objects() const;
    /**
     * Protected.
     * Retrieve the spatial-axis which has the largest range for all 2D objects in the Scene.
     * @return - The most variant spatial-axis
     */
    tz::physics::Axis2D get_highest_variance_axis_sprites() const;
    /**
     * Protected.
     * Erase the given StaticObject from the Scene instantly.
     * Note: This retains the difference in behaviour mentioned in Scene::remove_object(StaticObject&).
     * @param to_delete - The StaticObject to erase from the Scene
     */
    void erase_object(StaticObject* to_delete);
    /**
     * Protected.
     * Erase the given Sprite from the Scene instantly.
     * Note: This retains the difference in behaviour mentioned in Scene::remove_sprite(Sprite&).
     * @param to_delete - The Sprite to erase from the Scene
     */
    void erase_sprite(Sprite* to_delete);
    /**
     * Protected.
     * Handle all of the queued deletion requests made by Scene::remove_object(StaticObject&) and Scene::remove_sprite(Sprite&).
     * Note: If you are using Scene::erase_object(StaticObject*) and Scene::erase_sprite(Sprite*) instead, then this method is unneeded.
     */
    void handle_deletions();

    /// Container of all stack-allocated StaticObjects.
    std::vector<StaticObject> stack_objects;
    /// Container of all heap-allocated StaticObjects and all given subclasses, including DynamicObject.
    std::vector<std::unique_ptr<StaticObject>> heap_objects;
    /// Container of all stack-allocated Sprites.
    std::vector<Sprite> stack_sprites;
    /// Container of all heap-allocated Sprites and all given subclasses, including DynamciSprite.
    std::vector<std::unique_ptr<Sprite>> heap_sprites;
    /// Container of all DirectionalLights in the Scene.
    std::vector<DirectionalLight> directional_lights;
    /// Container of all PointLights in the Scene.
    std::vector<PointLight> point_lights;
    /// Container of all 3D objects that have been requested for deletion in the next Scene::update() invocation.
    std::vector<StaticObject*> objects_to_delete;
    /// Container of all 2D objects that have been requested for deletion in the next Scene::update() invocation.
    std::vector<Sprite*> sprites_to_delete;
};

#include "scene.inl"
#endif //TOPAZ_SCENE_HPP
