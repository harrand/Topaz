#include <type_traits>
#include "core/debug/assert.hpp"

namespace tz::gl
{

    template<typename Component>
    const Component& PixelRGBA<Component>::operator[](std::size_t idx) const
    {
        switch(idx)
        {
            case 0:
                return this->r;
            case 1:
                return this->g;
            case 2:
                return this->b;
            case 3:
                return this->a;
            default:
                topaz_assert(false, "PixelRGBA::operator[", idx, "] const. Value out of range!");
        }
        return this->r;
    }

    template<typename Component>
    Component& PixelRGBA<Component>::operator[](std::size_t idx)
    {
        switch(idx)
        {
            case 0:
                return this->r;
            case 1:
                return this->g;
            case 2:
                return this->b;
            case 3:
                return this->a;
            default:
                topaz_assert(false, "PixelRGBA::operator[", idx, "]. Value out of range!");
        }
        return this->r;
    }

    template<typename Component>
    bool PixelRGBA<Component>::operator==(const PixelRGBA<Component>& rhs) const
    {
        return this->r == rhs.r && this->g == rhs.g && this->b == rhs.b && this->a == rhs.a;
    }

    template<typename Component>
    bool PixelRGBA<Component>::operator!=(const PixelRGBA<Component>& rhs) const
    {
        return this->r != rhs.r || this->g != rhs.g || this->b != rhs.b || this->a != rhs.a;
    }

    template<typename Component>
    bool PixelRGB<Component>::operator==(const PixelRGB<Component>& rhs) const
    {
        return this->r == rhs.r && this->g == rhs.g && this->b == rhs.b;
    }

    template<typename Component>
    bool PixelRGB<Component>::operator!=(const PixelRGB<Component>& rhs) const
    {
        return this->r != rhs.r || this->g != rhs.g || this->b != rhs.b;
    }

    template<typename Component>
    const Component& PixelRGB<Component>::operator[](std::size_t idx) const
    {
        switch(idx)
        {
            case 0:
                return this->r;
            case 1:
                return this->g;
            case 2:
                return this->b;
            default:
                topaz_assert(false, "PixelRGB::operator[", idx, "]: Index out of range!");
        }
        return this->r;
    }

    template<typename Component>
    Component& PixelRGB<Component>::operator[](std::size_t idx)
    {
        switch(idx)
        {
            case 0:
                return this->r;
            case 1:
                return this->g;
            case 2:
                return this->b;
            default:
                topaz_assert(false, "PixelRGB::operator[", idx, "]: Index out of range!");
        }
        return this->r;
    }

    template<typename Component>
    bool PixelGrayscale<Component>::operator==(const PixelGrayscale<Component>& rhs) const
    {
        return this->c == rhs.c;
    }

    template<typename Component>
    bool PixelGrayscale<Component>::operator!=(const PixelGrayscale<Component>& rhs) const
    {
        return this->c != rhs.c;
    }

    template<typename Component>
    const Component& PixelGrayscale<Component>::operator[](std::size_t idx) const
    {
        switch(idx)
        {
            case 0:
                return this->c;
            default:
                topaz_assert(false, "PixelGrayscale::operator[", idx, "]: Index out of range!");
        }
        return this->c;
    }

    namespace pixel
    {
        template<typename Component>
        constexpr GLenum parse_component_type()
        {
            // Until c++20 template lambdas come, this is unfortunately going to be an ugly macro.
            #define chk(x, y) std::is_same_v<x, y>
            if constexpr(chk(Component, unsigned char) || chk(Component, std::byte))
            {
                return GL_UNSIGNED_BYTE;
            }
            else if constexpr(chk(Component, char))
            {
                return GL_BYTE;
            }
            else if constexpr(chk(Component, unsigned short))
            {
                return GL_UNSIGNED_SHORT;
            }
            else if constexpr(chk(Component, unsigned int))
            {
                return GL_UNSIGNED_INT;
            }
            else if constexpr(chk(Component, int))
            {
                return GL_INT;
            }
            else if constexpr(chk(Component, float))
            {
                return GL_FLOAT;
            }
            else
            {
                return GL_INVALID_VALUE;
            }
        }

        template<template<typename> class PixelType, typename ComponentType>
        constexpr GLint parse_internal_format()
        {
            #define chk(x, y) std::is_same_v<x, y>
            constexpr GLenum comp_type = tz::gl::pixel::parse_component_type<ComponentType>();
            if constexpr(chk(PixelType<ComponentType>, PixelRGBA<ComponentType>))
            {
                // Is some form of RGBA...
                // Let's find out specifically which!
                switch(comp_type)
                {
                    case GL_UNSIGNED_BYTE:
                    case GL_BYTE:  
                        return GL_RGBA8;
                        break;
                    case GL_FLOAT:
                        return GL_DEPTH_COMPONENT32;
                        break;
                }
            }
            return GL_INVALID_VALUE;
        }

        template<template<typename> class PixelType, typename ComponentType>
        constexpr GLenum parse_format()
        {
            #define chk(x, y) std::is_same_v<x, y>
            if constexpr(chk(PixelType<ComponentType>, PixelRGBA<ComponentType>))
            {
                return GL_RGBA;
            }
            else if constexpr(chk(PixelType<ComponentType>, PixelRGB<ComponentType>))
            {
                return GL_RGB;
            }
            else if constexpr(chk(PixelType<ComponentType>, PixelGrayscale<ComponentType>))
            {
                return GL_DEPTH_COMPONENT;
            }
            return GL_INVALID_VALUE;
        }
    }
}