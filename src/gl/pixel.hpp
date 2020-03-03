#ifndef TOPAZ_GL_PIXEL_HPP
#define TOPAZ_GL_PIXEL_HPP
#include <cstddef>
#include "glad/glad.h"

namespace tz::gl
{
    /**
     * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
     * @{
     */
    template<typename Component>
    struct PixelRGBA
    {
        static constexpr std::size_t num_components = 4;
        using ComponentType = Component;
        Component r;
        Component g;
        Component b;
        Component a;
        const Component& operator[](std::size_t idx) const;
        Component& operator[](std::size_t idx);
        bool operator==(const PixelRGBA<Component>& rhs) const;
        bool operator!=(const PixelRGBA<Component>& rhs) const;
    };

    template<typename Component>
    struct PixelRGB
    {
        static constexpr std::size_t num_components = 3;
        using ComponentType = Component;
        Component r;
        Component g;
        Component b;
        const Component& operator[](std::size_t idx) const;
        Component& operator[](std::size_t idx);
        bool operator==(const PixelRGB<Component>& rhs) const;
        bool operator!=(const PixelRGB<Component>& rhs) const;
    };

    template<typename Component>
    struct PixelRG
    {
        static constexpr std::size_t num_components = 2;
        using ComponentType = Component;
        Component x;
        Component y;
        const Component& operator[](std::size_t idx) const;
        Component& operator[](std::size_t idx);
        bool operator==(const PixelRG<Component>& rhs) const;
        bool operator!=(const PixelRG<Component>& rhs) const;
    };

    template<typename Component>
    struct PixelGrayscale
    {
        static constexpr std::size_t num_components = 1;
        using ComponentType = Component;
        Component c;

        const Component& operator[](std::size_t idx) const;
        Component& operator[](std::size_t idx);
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
    using PixelRGB8 = PixelRGB<std::byte>;
    using PixelRG8 = PixelRG<std::byte>;
    using PixelDepth = PixelGrayscale<float>;

    /**
     * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
     * @{
     */
}

#include "gl/pixel.inl"
#endif // TOPAZ_GL_PIXEL_HPP