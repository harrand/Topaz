#ifndef TOPAZ_SCENE_OBJECT_HPP
#define TOPAZ_SCENE_OBJECT_HPP

#include "graphics/mesh.hpp"
#include "graphics/asset.hpp"
#include "data/transform.hpp"
#include "physics/physics_object.hpp"
#include "graphics/renderable.hpp"

/**
 * StaticObjects do NOT own the assets that they use.
 * The assets provided MUST have a lifetime greater than or equal to the StaticObject.
 */
class StaticObject : public Renderable
{
public:
	/**
	 * Construct a StaticObject based upon a transform and asset.
	 * @param transform - The position, rotation, and scale of the object
	 * @param asset - The asset used to render the object
	 */
	StaticObject(Transform transform, Asset asset);
	StaticObject(const StaticObject& copy) = default;
	StaticObject(StaticObject&& move) = default;
	virtual ~StaticObject() = default;
	StaticObject& operator=(const StaticObject& rhs) = default;
	StaticObject& operator=(StaticObject&& rhs) = default;
	/**
	 * Get the underlying asset used to render this object
	 * @return - The objects asset
	 */
	const Asset& get_asset() const;
	/**
	 * Retrieve an AABB bounding the StaticObject, if it has a valid mesh or model.
	 * @return - Boundary bounding the StaticObject if geometry exists, null otherwise
	 */
	virtual std::optional<AABB> get_boundary() const override;
	/**
	 * Issue a render-call to draw the object.
	 * @param render_pass - The parameters with which to render the object
	 */
	virtual void render(RenderPass render_pass) const override;
	virtual std::unique_ptr<Renderable> unique_clone() const override;

	/// Underlying transform of the object. Can be edited by anyone at any time
	Transform transform;

	/**
	 * Shallow equality check between this object and another.
	 * This means that equality will succeed if and only if they have the exact same position, rotation, scale, and same elements in the same AssetBuffer.
	 * @param rhs - The object to compare to
	 * @return - True if the objects are shallowly equal, otherwise false
	 */
	bool operator==(const StaticObject& rhs) const;
	friend class InstancedStaticObject;
protected:
	/// Underlying asset.
	Asset asset;
};

/**
 * Container of StaticObjects specifically designed to be rendered in one draw-call. All objects must share deeply-identical assets.
 */
class InstancedStaticObject : public StaticObject
{
public:
	/**
	 * Construct an InstancedStaticObject based upon an existing set of objects
	 * @param objects - StaticObjects to batch an instance of
	 */
	InstancedStaticObject(const std::vector<StaticObject>& objects);
	/**
	 * Retrieve the underlying InstancedMesh.
	 * @return - Instanced mesh
	 */
	const InstancedMesh& get_instanced_mesh() const;
	/**
	 * Retrieve an AABB bounding all StaticObjects.
	 * @return - Boundary bounding all objects in the instanced object
	 */
	virtual std::optional<AABB> get_boundary() const override;
	/**
	 * Render the StaticObject instances in a single render-call.
	 * @param render_pass - Render properties of the render-call
	 */
	virtual void render(RenderPass render_pass) const override;
	virtual std::unique_ptr<Renderable> unique_clone() const override;
private:
	/// Underlying InstancedMesh.
	std::shared_ptr<InstancedMesh> instanced_mesh;
};

#endif //TOPAZ_SCENE_OBJECT_HPP
