//
// Created by Harrand on 10/08/2019.
//

#include "asteroid.hpp"
#include "asteroids.hpp"

Asteroid::Asteroid(float mass, Transform transform, Asset asset, Vector3F velocity, float moment_of_inertia, Vector3F angular_velocity, std::initializer_list<Vector3F> forces, AsteroidType type): DynamicObject(mass, transform, asset, velocity, moment_of_inertia, angular_velocity, forces), parent(nullptr), type(type){}

void Asteroid::on_collision([[maybe_unused]] PhysicsObject &collided)
{
    if(this->parent != nullptr)
    {
        switch(this->type)
        {
            case AsteroidType::GREEN_BULLET:
                this->parent->remove_object(*this);
                this->parent->remove_object(dynamic_cast<DynamicObject &>(collided));
                this->parent->play_boom();
                break;
            case AsteroidType::RED_BULLET:
                this->parent->remove_object(dynamic_cast<DynamicObject&>(collided));
                this->velocity /= -2.0f;
                this->clear_forces();
                this->add_force({0, -1000.0f, 0});
                this->parent->play_boom();
                break;
            case AsteroidType::BLUE_BULLET:
                this->parent->remove_object(dynamic_cast<DynamicObject&>(collided));
                this->velocity *= -1.0f;
                this->parent->play_boom();
                break;
            default:
                break;
        }

    }
}

void Asteroid::set_parent(Asteroids* scene)
{
    this->parent = scene;
}

void Asteroid::set_type(AsteroidType type)
{
    this->type = type;
}
