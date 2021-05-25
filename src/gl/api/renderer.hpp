#ifndef TOPAZ_GL_API_RENDERER_HPP
#define TOPAZ_GL_API_RENDERER_HPP
#include "core/containers/basic_list.hpp"
#include "core/vector.hpp"
#include "gl/impl/common/renderer.hpp"
#include "gl/render_pass.hpp"
#include "gl/shader.hpp"
#include <cstdint>
#include <concepts>

namespace tz::gl
{
    /**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * A collection of low-level renderer-agnostic graphical interfaces.
	 * @{
	 */
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
        tz::BasicList<RendererAttributeFormat> binding_attributes; // TODO: (C++20 constexpr std::vector support) replace with std::vector so we are a LiteralType. Then IRendererInput::get_format() can be constexpr.
    };

    /**
     * @brief A renderer is always provided some input data. This data is always sorted into vertex/index buffers eventually, but there may be custom setups where you need more control over how this data is represented in memory.
     * @details Renderer inputs can vary wildly in their nature depending on what sort of rendering you'd like to do. Topaz does not mandate a specific renderer input type, but the most common use-case is for storing mesh data. A class already exists for this purpose: @ref MeshInput
     * 
     * @pre IRendererInput declares `IRendererInput::unique_clone()` which derived types must implement. If you are implementing derived type `D`, if `D` is copy-constructible then implement `IRendererInputCopyable<D>` instead. If `D` is not copy-constructible then you must implement `IRendererInput::unique_clone()` yourself.
     * 
     */
    class IRendererInput
    {
    public:
        /**
         * @brief Clone the renderer input, creating an exact copy of the input data.
         * 
         * @return Smart pointer to the new IRendererInput. Ownership must be claimed by the caller.
         */
        [[nodiscard]] virtual std::unique_ptr<IRendererInput> unique_clone() const = 0;

        virtual constexpr RendererInputDataAccess data_access() const = 0;
        virtual RendererElementFormat get_format() const = 0;
        virtual std::span<const std::byte> get_vertex_bytes() const = 0;
        virtual std::span<const unsigned int> get_indices() const = 0;
    };

    /**
     * @brief Identical to @ref IRendererInput, but `IRendererInputCopyable<T>::unique_clone()` need not be implemented.
     * @pre Derived must be copy-constructible. Otherwise, the program is ill-formed.
     * 
     * @tparam Derived Renderer input type. It must be copy-constructible.
     */
    template<class Derived>
    class IRendererInputCopyable : public IRendererInput
    {
    public:
        /// Invokes `Derived::Derived(const Derived&)`
        [[nodiscard]] virtual std::unique_ptr<IRendererInput> unique_clone() const final
        {
            static_assert(requires{requires std::copyable<Derived>;}, "IRendererInputCopyable<T>: T must be copyable. Derive from IRendererInput and implement unique_clone if not copyable.");
            return std::make_unique<Derived>(static_cast<const Derived&>(*this));
        }
    };

    class IRendererDynamicInput : public IRendererInput
    {
    public:
        virtual constexpr RendererInputDataAccess data_access() const final{return RendererInputDataAccess::DynamicFixed;}

        virtual std::span<std::byte> get_vertex_bytes_dynamic() = 0;
        virtual void set_vertex_data(std::byte* vertex_data) = 0;
        virtual void set_index_data(unsigned int* index_data) = 0;
    };

    /**
     * @brief Identical to @ref IRendererInput, but `IRendererInputCopyable<T>::unique_clone()` need not be implemented.
     * @pre Derived must be copy-constructible. Otherwise, the program is ill-formed.
     * 
     * @tparam Derived Renderer input type. It must be copy-constructible.
     */
    template<class Derived>
    class IRendererDynamicInputCopyable : public IRendererDynamicInput
    {
    public:
        /// Invokes `Derived::Derived(const Derived&)`
        [[nodiscard]] virtual std::unique_ptr<IRendererInput> unique_clone() const final
        {
            static_assert(requires{requires std::copyable<Derived>;}, "IRendererInputCopyable<T>: T must be copyable. Derive from IRendererInput and implement unique_clone if not copyable.");
            return std::make_unique<Derived>(static_cast<const Derived&>(*this));
        }
    };

    class IRendererOutput
    {
    public:
        virtual ~IRendererOutput() = default;
        virtual void set_render_target() const = 0;
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
         * @brief Provide initial input data for the renderer.
         * @todo Allow this data to be changed later. Right now as soon as the Renderer is constructed this data never changes.
         * 
         * @param input Reference to an existing @ref IRendererInput
         */
        virtual void set_input(const IRendererInput& input) = 0;
        /**
         * @brief Retrieve the format of the vertex data elements.
         * 
         * @return RendererElementFormat describing how vertex data is laid out in memory.
         */
        virtual const IRendererInput* get_input() const = 0;

        virtual void set_output(const IRendererOutput& output) = 0;
        virtual const IRendererOutput* get_output() const = 0;

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

        virtual void set_shader(const Shader& shader) = 0;
        virtual const Shader& get_shader() const = 0;
    };

    class IRenderer
    {
    public:
        /**
         * @brief Set the clear colour for any colour attachments throughout the renderpass.
         * 
         * @param clear_colour Clear colour value, as a normalised Vec4.
         */
        virtual void set_clear_colour(tz::Vec4 clear_colour) = 0;
        /**
         * @brief Retrieve the clear colour for any colour attachments throughout the renderpass.
         * @note The default clear colour is {0, 0, 0, 0} (solid black) on all platforms.
         * 
         * @return Clear colour value, as a normalised Vec4.
         */
        virtual tz::Vec4 get_clear_colour() const = 0;

        virtual IRendererInput* get_input() = 0;

        virtual void render() = 0;
    };
    /**
     * @}
     */
}

#endif // TOPAZ_GL_API_RENDERER_HPP