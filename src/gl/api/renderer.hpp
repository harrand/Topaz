#ifndef TOPAZ_GL_API_RENDERER_HPP
#define TOPAZ_GL_API_RENDERER_HPP
#include "core/containers/basic_list.hpp"
#include "gl/impl/common/renderer.hpp"
#include "gl/render_pass.hpp"
#include <cstdint>

namespace tz::gl
{
    /**
     * @brief Describes the layout of a vertex attribute in memory.
     */
    struct RendererAttributeFormat
    {
        /// How many bytes between the beginning of the element and this attribute location (offsetof)
        std::size_t element_attribute_offset;
        /// What is the data type of this attribute?
        RendererComponentType type;
    };

    /**
     * @brief Describes the nature of the vertex data in memory. Vertex data is organised as an array of elements. Each element has one or more attributes. 
     */
    struct RendererElementFormat
    {
        /// What is the total size of this element? Includes all attributes and any padding.
        std::size_t binding_size;
        /// How often should we expect to see these elements? Per vertex, or per instance?
        RendererInputFrequency basis;
        /// List of all attributes. These must be in order.
        tz::BasicList<RendererAttributeFormat> binding_attributes;
    };

    /**
     * @brief Structure describing the nature of a renderer.
     * @note There is no default element format. You must specify one before creating a renderer, otherwise the behaviour is undefined.
     * @note The default culling strategy is no culling. You are likely to improve performance by utilising a culling strategy.
     */
    class IRendererBuilder
    {
    public:
        /**
         * @brief Set the format of the vertex data elements.
         * 
         * @param element_format RendererElementFormat describing the layout of the vertex data in memory.
         */
        virtual void set_element_format(RendererElementFormat element_format) = 0;
        /**
         * @brief Retrieve the format of the vertex data elements.
         * 
         * @return RendererElementFormat describing how vertex data is laid out in memory.
         */
        virtual RendererElementFormat get_element_format() const = 0;
        /**
         * @brief Set the culling strategy used during rendering.
         * 
         * @param culling_strategy Which faces of elements should be culled during rendering?
         */
        virtual void set_culling_strategy(RendererCullingStrategy culling_strategy) = 0;
        /**
         * @brief Retrieve the current culling strategy.
         * 
         * @return Culling strategy that the renderer will use.
         */
        virtual RendererCullingStrategy get_culling_strategy() const = 0;
        /**
         * @brief Renderers must reference an existing RenderPass. Renderers will render each stage of the render pass in the expected order.
         * 
         * @param render_pass Render pass that will be ran
         */
        virtual void set_render_pass(const RenderPass& render_pass) = 0;
        /**
         * @brief Retrieve the existing render pass associated with this renderer.
         * @pre A render pass must have previously been associated with this renderer via IRendererBuilder::set_render_pass
         * 
         * @return The current render pass object referenced by this renderer.
         */
        virtual const RenderPass& get_render_pass() const = 0;
    };

    class IRenderer
    {
    public:

    };
}

#endif // TOPAZ_GL_API_RENDERER_HPP