#ifndef TOPAZ_GL_TRANSFORM
#define TOPAZ_GL_TRANSFORM
#include "geo/vector.hpp"

namespace tz::gl
{
    struct Transform
    {
        tz::Vec3 position = {};
        tz::Vec3 rotation = {};
        tz::Vec3 scale = tz::Vec3{1.0f, 1.0f, 1.0f};
    };
}

#endif // TOPAZ_GL_TRANSFORM