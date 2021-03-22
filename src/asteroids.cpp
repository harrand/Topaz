//
// Created by Harrand on 10/08/2019.
//

#include "asteroids.hpp"

Asteroids::Asteroids(): Scene(), assets(), shoot("../res/runtime/music/shoot.wav"), shot_impact("../res/runtime/music/shot_impact.wav")
{
    assets.emplace<Mesh>("sphere", "../res/runtime/models/sphere.obj");

    assets.emplace<Texture>("igneous", "../res/runtime/textures/lava.jpg");
    assets.emplace<NormalMap>("igneous", "../res/runtime/normalmaps/lava_normalmap.jpg");

    assets.emplace<Texture>("sedimentary", "../res/runtime/textures/sand.jpg");
    assets.emplace<NormalMap>("sedimentary", "../res/runtime/normalmaps/sand_normalmap.jpg");

    assets.emplace<Texture>("metallic", "../res/runtime/textures/metal.jpg");
    assets.emplace<NormalMap>("metallic", "../res/runtime/textures/metal_normalmap.jpg");
}

Asteroid& Asteroids::emplace_asteroid(Transform transform, AsteroidType type, Vector3F velocity, Vector3F angular_velocity)
{
    Asteroid& ret =  this->emplace<Asteroid>(1.0f, transform, this->resolve_asteroid(type), velocity, 1.0f, angular_velocity);
    ret.make_collisions = false;
    ret.set_parent(this);
    ret.set_type(type);
    return ret;
}

void Asteroids::play_boom()
{
    this->shot_impact.play();
}

void Asteroids::play_shoot()
{
    this->shoot.play();
}

Asset Asteroids::resolve_asteroid(AsteroidType type)
{
    Asset result{assets.find_mesh("sphere"), nullptr};
    const char* type_str;
    switch(type)
    {
        case AsteroidType::IGNEOUS:
            type_str = "igneous";
            break;
        case AsteroidType::SEDIMENTARY:
            type_str = "sedimentary";
            break;
        default:
        case AsteroidType::METALLIC:
            type_str = "metallic";
            break;
    }
    result.texture = assets.find_texture(type_str);
    result.normal_map = assets.find<NormalMap>(type_str);
    return result;
}