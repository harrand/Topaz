#ifndef TOPAZ_LIGHT_HPP
#define TOPAZ_LIGHT_HPP

#include "data/matrix.hpp"
#include "graphics/camera.hpp"
#include "physics/boundary.hpp"

/**
 * A generic Light object.
 */
class Light
{
public:
    /**
     * Construct a generic Light with given specifications.
     * @param colour - Colour of the light, in RGB. Clamped between 0.0 and 1.0 per component
     * @param power - Wattage of the light
     */
    Light(Vector3F colour = {1, 1, 1}, float power = 1.0f);
    /**
     * Get the current colour of the Light.
     * @return - The colour of the light, in RGB. Clamped between 0.0 and 1.0 per component
     */
    const Vector3F& get_colour()const;
    /**
     * Get the power of the Light.
     * @return - Power, in Watts
     */
    float get_power() const;
    /**
     * Set the colour of this Light.
     * @param colour - RGB format. Clamped between 0.0 and 1.0 per component
     */
    void set_colour(Vector3F colour);
    /**
     * Set the power of this Light.
     * @param power - Power, in Watts
     */
    void set_power(float power);
private:
    /// Colour of the Light, in RGB (0.0 to 1.0).
    Vector3F colour;
    /// Power of the Light, in Watts.
    float power;
};

/**
 * A Directional Light. A Directional Light is infinitely far away whose light-rays are parallel to each other (such as a Sun).
 */
class DirectionalLight : public Light
{
public:
    /**
     * Construct a DirectionalLight using the given specifications.
     * @param direction - Direction that the light-rays should go toward
     * @param colour - Colour of the light, in RGB (0.0 to 1.0)
     * @param power - Power of each light ray, in Watts. There is no attenuation to DirectionalLights
     */
    DirectionalLight(Vector3F direction = {}, Vector3F colour = {1, 1, 1}, float power = 1.0f);
    /**
     * Get the direction of the light-rays.
     * @return - Direction of the light-rays.
     */
    const Vector3F& get_direction() const;
    /**
     * Set the desired direction of the light-rays of this DirectionalLight.
     * @param direction - Desired direction of the light-rays.
     */
    void set_direction(Vector3F direction);
    /**
     * Retrieve a virtual Camera, representing the point of view of the Light.
     * Note: DirectionalLight views use a orthographic projection.
     * @param scene_boundary - An AABB containing the entire Scene
     * @return - The virtual Camera
     */
    Camera get_view(const AABB& scene_boundary) const;
private:
    /// Direction of the light-rays.
    Vector3F direction;
};

/**
 * A Point Light. A Point Light sits in a given location and attenuates as the distance from the location increases.
 */
class PointLight : public Light
{
public:
    /**
     * Construct a PointLight using the given specifications.
     * @param position - Position of the centre of the Light, in world-space
     * @param colour - Colour of the light, in RGB (0.0, 1.0)
     * @param power - Power of the light, in Watts
     */
    PointLight(Vector3F position = {}, Vector3F colour = {1, 1, 1}, float power = 1.0f);
    /// Position of the centre of the Point Light.
    Vector3F position;
};


#endif //TOPAZ_LIGHT_HPP
