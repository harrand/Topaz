//
// Created by Harry on 24/12/2018.
//

#ifndef TOPAZ_GEOMETRY_HPP
#define TOPAZ_GEOMETRY_HPP

#include "data/vector.hpp"
#include <memory>

/// Abstract 2D shape
class Polygon
{
public:
    Polygon();
    virtual float get_area() const = 0;
};

class Rectangle : public Polygon
{
public:
    Rectangle(Vector2F dimensions);
    const Vector2F& get_dimensions() const;
    // Will absolute away any negatives
    void set_dimensions(const Vector2F& dimensions);
    virtual float get_area() const override;
protected:
    Vector2F dimensions;
};

class Circle : public Polygon
{
public:
    Circle(float radius);
    float get_radius() const;
    void set_radius(float radius);
    virtual float get_area() const override;
    float get_circumference() const;
protected:
    float radius;
};

/// Abstract 3D shape
class Polyhedron
{
public:
    virtual float get_surface_area() const = 0;
    virtual float get_volume() const = 0;
};

class Cuboid : public Polyhedron
{
public:
    Cuboid(Vector3F dimensions);
    const Vector3F& get_dimensions() const;
    void set_dimensions(const Vector3F& dimensions);
    virtual float get_surface_area() const override;
    virtual float get_volume() const override;
protected:
    Vector3F dimensions;
};

class Sphere : public Polyhedron
{
public:
    Sphere(float radius);
    float get_radius() const;
    void set_radius(float radius);
    virtual float get_surface_area() const override;
    virtual float get_volume() const override;
protected:
    float radius;
};

class Plane : public Polyhedron
{
public:
    Plane(Vector3F normal, std::optional<Vector2F> finite_dimensions = std::nullopt);
    bool is_finite() const;
    std::optional<Vector2F> get_dimensions() const;
    void set_infinite();
    void set_finite(Vector2F finite_dimensions);
    virtual float get_surface_area() const override;
    virtual float get_volume() const override;

    Vector3F normal;
private:
    std::optional<Vector2F> finite_dimensions;
};

#endif //TOPAZ_GEOMETRY_HPP
