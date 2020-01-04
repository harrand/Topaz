#ifndef TOPAZ_GL_PIXEL_HPP
#define TOPAZ_GL_PIXEL_HPP
#include <cstddef>
#include "glad/glad.h"

namespace tz::gl
{
    template<typename Component>
    struct PixelRGBA
    {
        using ComponentType = Component;
        Component r;
        Component g;
        Component b;
        Component a;
        bool operator==(const PixelRGBA<Component>& rhs) const;
        bool operator!=(const PixelRGBA<Component>& rhs) const;
    };

    template<typename Component>
    struct PixelRGB
    {
        using ComponentType = Component;
        Component r;
        Component g;
        Component b;
        bool operator==(const PixelRGB<Component>& rhs) const;
        bool operator!=(const PixelRGB<Component>& rhs) const;
    };

    template<typename Component>
    struct PixelGrayscale
    {
        using ComponentType = Component;
        Component c;

        bool operator==(const PixelGrayscale<Component>& rhs) const;
        bool operator!=(const PixelGrayscale<Component>& rhs) const;
    };

    namespace pixel
    {
        template<typename Component>
        constexpr GLenum parse_component_type();
        template<template<typename> class PixelType, typename ComponentType>
        constexpr GLint parse_internal_format();
        template<template<typename> class PixelType, typename ComponentType>
        constexpr GLenum parse_format();
    }
    using PixelRGBA8 = PixelRGBA<std::byte>;
    using PixelDepth = PixelGrayscale<float>;
}

#include "gl/pixel.inl"
#endif // TOPAZ_GL_PIXEL_HPP