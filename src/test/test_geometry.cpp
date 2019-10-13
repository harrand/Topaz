//
// Created by Harry on 30/12/2018.
//

#define UNIT_TEST "Geometry"
#include "lib/test_util.hpp"
#include "utility/geometry.hpp"

void test_polygons()
{
    // Rectangle
    Rectangle rect{{10, 10}};
    tz::assert::equal(rect.get_area(), 100.0f);
    tz::assert::equal<Vector2F>(rect.get_dimensions(), Vector2F{10.0f, 10.0f});
    rect.set_dimensions({20, 5});
    tz::assert::equal<Vector2F>(rect.get_dimensions(), Vector2F{20.0f, 5.0f});

    Circle circ{1.0f};
    tz::assert::equal(circ.get_radius(), 1.0f);
    float pi = static_cast<float>(tz::consts::numeric::pi);
    tz::assert::equal(circ.get_circumference(), 2.0f * pi);
    tz::assert::equal(circ.get_area(), pi);
    circ.set_radius(50.0f);
    tz::assert::equal(circ.get_radius(), 50.0f);
    tz::assert::equal(circ.get_circumference(), 100.0f * pi);
    tz::assert::equal<int>(circ.get_area(), static_cast<int>(2500.0f * pi));

    // Generic Polygon
    Polygon& rectangle = rect;
    Polygon& circle = circ;
    tz::assert::equal(rectangle.get_area(), rect.get_area());
    tz::assert::equal(circle.get_area(), circ.get_area());
}

void test_polyhedra()
{

}

void test()
{
    test_polygons();
    test_polyhedra();
}