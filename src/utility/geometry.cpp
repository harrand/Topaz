//
// Created by Harry on 24/12/2018.
//

#include "geometry.hpp"

Polygon::Polygon(){}

Rectangle::Rectangle(Vector2F dimensions): Polygon(), dimensions(dimensions)
{
	this->set_dimensions(dimensions);
}

const Vector2F& Rectangle::get_dimensions() const
{
	return this->dimensions;
}

void Rectangle::set_dimensions(const Vector2F& dimensions)
{
	this->dimensions.x = std::abs(dimensions.x);
	this->dimensions.y = std::abs(dimensions.y);
}

float Rectangle::get_area() const
{
	return this->dimensions.x * this->dimensions.y;
}

Circle::Circle(float radius): Polygon(), radius(radius)
{
	this->set_radius(radius);
}

float Circle::get_radius() const
{
	return this->radius;
}

void Circle::set_radius(float radius)
{
	this->radius = std::abs(radius);
}

float Circle::get_area() const
{
	return static_cast<float>(tz::utility::numeric::consts::pi * std::pow(this->radius, 2));
}

float Circle::get_circumference() const
{
	return static_cast<float>(2.0 * tz::utility::numeric::consts::pi * this->radius);
}

Cuboid::Cuboid(Vector3F dimensions): dimensions(dimensions)
{
	this->set_dimensions(dimensions);
}

const Vector3F& Cuboid::get_dimensions() const
{
	return this->dimensions;
}

void Cuboid::set_dimensions(const Vector3F& dimensions)
{
	this->dimensions.x = std::abs(dimensions.x);
	this->dimensions.y = std::abs(dimensions.y);
	this->dimensions.z = std::abs(dimensions.z);
}

float Cuboid::get_surface_area() const
{
	return 2.0f * this->dimensions.z * this->dimensions.x + 2.0f * this->dimensions.z * this->dimensions.y + 2.0f * this->dimensions.y * this->dimensions.x;
}

float Cuboid::get_volume() const
{
	return this->dimensions.x * this->dimensions.y * this->dimensions.z;
}

Sphere::Sphere(float radius): radius(radius)
{
	this->set_radius(radius);
}

float Sphere::get_radius() const
{
	return this->radius;
}

void Sphere::set_radius(float radius)
{
	this->radius = std::abs(radius);
}

float Sphere::get_surface_area() const
{
	return static_cast<float>(4.0 * tz::utility::numeric::consts::pi * std::pow(this->radius, 2));
}

float Sphere::get_volume() const
{
	return static_cast<float>(4.0 / 3.0 * tz::utility::numeric::consts::pi * std::pow(this->radius, 3));
}

Plane::Plane(Vector3F normal, std::optional<Vector2F> finite_dimensions): normal(normal), finite_dimensions(finite_dimensions){}

bool Plane::is_finite() const
{
	return this->finite_dimensions.has_value();
}

std::optional<Vector2F> Plane::get_dimensions() const
{
	return this->finite_dimensions;
}

void Plane::set_infinite()
{
	this->finite_dimensions = std::nullopt;
}

void Plane::set_finite(Vector2F finite_dimensions)
{
	this->finite_dimensions = finite_dimensions;
}

float Plane::get_surface_area() const
{
	if(this->is_finite())
		return this->finite_dimensions.value().x * this->finite_dimensions.value().y;
	else
		return std::numeric_limits<float>::max();
}

float Plane::get_volume() const
{
	return this->get_surface_area();
}