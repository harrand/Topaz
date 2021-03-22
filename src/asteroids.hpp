//
// Created by Harrand on 10/08/2019.
//

#ifndef TOPAZ_ASTEROIDS_HPP
#define TOPAZ_ASTEROIDS_HPP
#include "core/scene.hpp"
#include "asteroid.hpp"

class Asteroids : public Scene
{
public:
    Asteroids();
    Asteroid& emplace_asteroid(Transform transform, AsteroidType type, Vector3F velocity = {}, Vector3F angular_velocity = {});
    void play_boom();
    void play_shoot();
private:
    Asset resolve_asteroid(AsteroidType type);

    AssetBuffer assets;
    AudioClip shoot, shot_impact;
    bool need_shoot;
};


#endif //TOPAZ_ASTEROIDS_HPP
