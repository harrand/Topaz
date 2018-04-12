#include "object.hpp"

#ifndef SPRITE_HPP
#define SPRITE_HPP

/**
* Like a normal Object2D, but has a texture bound.
*/
class Sprite : public Object2D
{
public:
    /**
     * Construct a Sprite with all specifications, using a Texture.
     * @param position - 2-dimensional Vector representing the position of the Object2D, in world-space
	 * @param rotation - Scalar representing the orientation of the Object2D, in radians
	 * @param scale - 2-dimensional Vector representing the scale of the Object2D in each spatial dimension
     * @param texture - Pointer to the Texture to bind to this Sprite
     */
    Sprite(Vector2F position, float rotation, Vector2F scale, const Texture* texture);
    /**
     * Construct a Sprite with all specifications, using a colour.
     * @param position - 2-dimensional Vector representing the position of the Object2D, in world-space
	 * @param rotation - Scalar representing the orientation of the Object2D, in radians
	 * @param scale - 2-dimensional Vector representing the scale of the Object2D in each spatial dimension
     * @param colour - Colour of the Sprite
     */
    Sprite(Vector2F position, float rotation, Vector2F scale, Vector4F colour);
    /**
     * Assign this Sprite to be equal to another.
     * @param copy - The Sprite to copy
     * @return - This, after assignment
     */
    Sprite& operator=(const Sprite& copy);
    /**
	 * Render this Sprite, through an existing Camera via an existing Shader
	 * @param cam - The Camera through which to view the Sprite
	 * @param shader - The Shader with which to render the Sprite
	 * @param width - The width of the viewport, in any unit proportional to pixels
	 * @param height - The height of the viewport, in any unit proportional to pixels
	 */
    virtual void render(const Camera& cam, Shader* shader, float width, float height) const override;
private:
    /// The Texture with which to render this Sprite. Uses colour instead if nullptr.
    const Texture* texture;
};


#endif //SPRITE_HPP
