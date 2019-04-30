//
// Created by Harrand on 12/04/2019.
//

#ifndef TOPAZ_RENDER_HPP
#define TOPAZ_RENDER_HPP
#include "graphics/static_object.hpp"
#include "physics/boundary.hpp"

/**
 * Like any other StaticObject, but always toggles wireframe-mode between render calls so that the box is transparent.
 */
class RenderableBoundingBox : public StaticObject
{
public:
	/**
	 * Construct a RenderableBoundingBox as normal.
	 * @param transform - The position, rotation, and scale of the object
	 * @param asset - The asset used to render the object
	 */
	RenderableBoundingBox(Transform transform, Asset asset, float wireframe_width = 1.0f);
	/**
	 * Issue a render-call to draw the object.
	 * @param render_pass - The parameters with which to render the object
	 */
	virtual void render(RenderPass render_pass) const override;
	virtual std::unique_ptr<Renderable> unique_clone() const override;
private:
	float wire_width;
};

namespace tz::utility::render
{
	/**
	 * Given an AABB, construct a RenderableBoundingBox representing the AABB.
	 * The colour passed is used to store a bitmap of dimensions 1x1 in the buffer (if one does not already exist), with name corresponding to the colour.
	 * @param buffer - Buffer to store the bitmap inside
	 * @param box - AABB to represent
	 * @param colour - Colour of the renderable AABB
	 * @return - The StaticObject equivalent of the given AABB
	 */
	RenderableBoundingBox see_aabb(AssetBuffer& buffer, const AABB& box, const Vector3F& colour = {1.0f, 1.0f, 1.0f}, float wire_width = 1.0f);
}

#endif //TOPAZ_RENDER_HPP
