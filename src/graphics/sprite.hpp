
#ifndef TOPAZ_SPRITE_HPP
#define TOPAZ_SPRITE_HPP
#include "graphics/mesh.hpp"
#include "graphics/texture.hpp"
#include "graphics/renderable.hpp"

/**
 * Similar to StaticObject, but always has a trivial plane for its mesh. Used to represent 2D sprites.
 */
class Sprite : public Renderable
{
public:
	/**
	 * Construct a Sprite with the given properties.
	 * @param position_screenspace - Position on the screen, in pixels
	 * @param rotation - Rotation of the sprite, in radians
	 * @param scale - Scale of the sprite
	 * @param texture - Texture attached to the sprite for rendering
	 */
	Sprite(Vector2F position_screenspace, float rotation, Vector2F scale, const Texture* texture);
	Sprite(const Sprite& copy) = default;
	Sprite(Sprite&& move) = default;
	virtual ~Sprite() = default;
	Sprite& operator=(const Sprite& rhs) = default;
	Sprite& operator=(Sprite&& rhs) = default;

	virtual std::optional<AABB> get_boundary() const override{return {std::nullopt};}
	/**
	 * Issue a render-call to draw the sprite.
	 * @param render_pass - The parameters of this specific rendering call.
	 */
	virtual void render(RenderPass render_pass) const override;
	virtual std::unique_ptr<Renderable> unique_clone() const override;
	/**
	 * Retrieve the clockwise rotation of the sprite, in radians.
	 * @return - Rotation in radians
	 */
	float get_rotation() const;
	/**
	 * Set the clockwise rotation of the sprite, in radians.
	 * @param radians - Rotation, in radians
	 */
	void set_rotation(float radians);
	/**
	 * Retrieve the current texture attached to this sprite
	 * @return - Texture currently used to render this sprite
	 */
	const Texture* get_texture() const;
	/**
	 * Attach the given texture to this sprite.
	 * @param texture - The texture with which this sprite should be rendered
	 */
	void set_texture(const Texture* texture);

	/**
	 * Compare this sprite with another. This will succeed if they have the same position, rotation and deeply-equal textures.
	 * @param rhs
	 * @return
	 */
	bool operator==(const Sprite& rhs) const;
	/// Position of the sprite, in screen-space.
	Vector2F position_screenspace;
protected:
	/// Rotation of the sprite, in radians.
	float rotation;
public:
	/// Scale of the sprite.
	Vector2F scale;
protected:
	/// Current texture attached to the sprite.
	const Texture* texture;
	/// Mesh used to render the sprite. It will be a simple plane mesh upon construction.
	Mesh mesh;
};


#endif //TOPAZ_SPRITE_HPP
