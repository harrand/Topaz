#ifndef TOPAZ_GL_API_RENDERER_HPP
#define TOPAZ_GL_API_RENDERER_HPP
#include "core/containers/basic_list.hpp"
#include "gl/impl/common/renderer.hpp"
#include <cstdint>

namespace tz::gl
{
    struct RendererAttributeFormat
    {
        std::size_t element_attribute_offset;
        RendererComponentType type;
    };

    struct RendererElementFormat
    {
        std::size_t binding_size;
        RendererInputFrequency basis;
        tz::BasicList<RendererAttributeFormat> binding_attributes;
    };

    class IRendererBuilder
    {
    public:
        virtual void set_element_format(RendererElementFormat element_format) = 0;
        virtual RendererElementFormat get_element_format() const = 0;
        virtual void set_culling_strategy(RendererCullingStrategy culling_strategy) = 0;
        virtual RendererCullingStrategy get_culling_strategy() const = 0;
    };

    class IRenderer
    {
    public:

    };
}

#endif // TOPAZ_GL_API_RENDERER_HPP