#ifndef TOPAZ_GL_API_RENDERER_HPP
#define TOPAZ_GL_API_RENDERER_HPP
#include "core/containers/basic_list.hpp"
#include "core/interfaces/cloneable.hpp"
#include "core/vector.hpp"
#include "gl/impl/common/renderer.hpp"
#include "gl/render_pass.hpp"
#include "gl/resource.hpp"
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
    class IRendererInput : public tz::IUniqueCloneable<IRendererInput>
    {
    public:
        virtual ~IRendererInput() = default;
        /**
         * @brief Retrieve the data access specifier for this render input type.
         * @note Inputs derived from @ref IRendererInput are `StaticFixed` by default, but this can be overriden. Inputs derived from @ref IRendererDynamicInput are always `DynamicFixed` and this cannot be overridden.
         * 
         * @return Access specifier for the data relative to the @ref IRenderer.
         */
        virtual constexpr RendererInputDataAccess data_access() const {return RendererInputDataAccess::StaticFixed;}
        /**
         * @brief Obtain the format of the input elements.
         * 
         * @return RenderElementFormat corresponding to layout of a single vertex data element.
         */
        virtual RendererElementFormat get_format() const = 0;
        /**
         * @brief Retrieve the vertex data as bytes. The data within the span is immutable.
         * @note See @ref IRendererDynamicInput::get_vertex_bytes_dynamic() for the option of mutable vertex data.
         * @note It is vaild to interpret these bytes as a `VertexType`, where VertexType is the type of the vertex data.
         * @return std::span<const std::byte> displaying the byte-representation of vertex data.
         */
        virtual std::span<const std::byte> get_vertex_bytes() const = 0;
        /**
         * @brief Retrieve the index data.
         * 
         * @return std::span<const unsigned int> displaying an array of all the indices.
         */
        virtual std::span<const unsigned int> get_indices() const = 0;
        std::size_t vertex_count() const
        {
            return this->vertex_count_bytes() / this->get_format().binding_size;
        }
        std::size_t vertex_count_bytes() const
        {
            return this->get_vertex_bytes().size_bytes();
        }
        std::size_t index_count() const
        {
            return this->get_indices().size();
        }
        std::size_t index_count_bytes() const
        {
            return this->get_indices().size_bytes();
        }
    };

    /// Opaque handle which can be provided to @ref IRenderer::get_input() to retrieve an input.
    using RendererInputHandle = tz::Handle<IRendererInput>;

    /**
     * @details RendererDrawLists contain a permutation of renderer inputs which will be drawn in a given render invocation. Typically, you can use a draw list to represent objects in a 3D scene.
     * 
     * Let's say for example you're building a fantasy game. You would like to draw three goblins within the world. The goblin mesh is stored within a @ref IRenderer as an input, with the input handle `x`.
     * 
     * You will need to draw the same input `x` three times. Your draw list may look like: `{x, x, x}`. Note that this will only draw the goblin mesh three times, and none of the other inputs. The draw list should correspond exactly with every object you would like to draw for the given frame. Do not neglect the concept of draw lists as the default draw-list is rarely useful. See @ref IRenderer::render() for information about default draw lists.
     * 
     */
    using RendererDrawList = tz::BasicList<tz::gl::RendererInputHandle>;

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
            static_assert(std::is_copy_constructible_v<Derived>, "IRendererInputCopyable<T>: T must be copyable. Derive from IRendererInput and implement unique_clone if not copyable.");
            return std::make_unique<Derived>(static_cast<const Derived&>(*this));
        }
    };

    /**
     * @brief Similar to @ref IRendererInput, but the vertex/index data can be changed at any point, even while used by a @ref IRenderer.
     */
    class IRendererDynamicInput : public IRendererInput
    {
    public:
        /**
         * @brief Retrieve the data access specifier for this render input type.
         * @note Inputs derived from @ref IRendererInput are `StaticFixed` by default. Inputs derived from @ref IRendererDynamicInput are always `DynamicFixed`.
         * 
         * @return constexpr RendererInputDataAccess 
         */
        virtual constexpr RendererInputDataAccess data_access() const final{return RendererInputDataAccess::DynamicFixed;}
        /**
         * @brief Retrieve the vertex data as bytes. The data within the span is mutable.
         * @note Aside from mutablility, this is functionally identical to @ref IRendererInput::get_vertex_bytes().
         * @note Dynamic vertex data can be edited on-the-fly -- It is valid to edit data even while the input is in-use by a renderer, in which case the updated values are guaranteed to be visible in the next render invocation.
         * @return std::span<std::byte> displaying the byte-representation of vertex data.
         */
        virtual std::span<std::byte> get_vertex_bytes_dynamic() = 0;
        
        #if TZ_VULKAN
            friend class RendererBufferManagerVulkan;
        #elif TZ_OGL
            friend class RendererOGL;
        #endif
    private:
        // Only intended to be used by the Renderer.
        virtual void set_vertex_data(std::byte* vertex_data) = 0;
        // Only intended to be used by the Renderer.
        virtual void set_index_data(unsigned int* index_data) = 0;
    };

    /**
     * @brief Identical to @ref IRendererDynamicInput, but `IRendererDynamicInputCopyable<T>::unique_clone()` need not be implemented.
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
            static_assert(std::is_copy_constructible_v<Derived>, "IRendererInputCopyable<T>: T must be copyable. Derive from IRendererInput and implement unique_clone if not copyable.");
            return std::make_unique<Derived>(static_cast<const Derived&>(*this));
        }
    };

    class IRendererOutput
    {
    public:
        virtual ~IRendererOutput() = default;
        virtual RendererOutputType get_type() const = 0;
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
         * @brief Add input data for the renderer.
         * When a Renderer performs a render, the data within each input is expressed as a list of vertex data and indices. If multiple inputs have been added, they will all be drawn in as few draw-calls as possible.
         * 
         * It is an error to retain this reference to dynamic input data and expect to change it later. To do that, create the Renderer as normal and invoke @ref IRenderer::get_input() to retrieve the Renderer's own copy of the input and perform your processing there.
         * @note When the Renderer is constructed, it will have its own copy of the input.
         * 
         * @param input Reference to an existing @ref IRendererInput
         */
        virtual RendererInputHandle add_input(const IRendererInput& input) = 0;
        /**
         * @brief Retrieve the format of the vertex data elements.
         * 
         * @return RendererElementFormat describing how vertex data is laid out in memory.
         */
        virtual const IRendererInput* get_input(RendererInputHandle input_handle) const = 0;

        virtual void set_output(const IRendererOutput& output) = 0;
        virtual const IRendererOutput* get_output() const = 0;

        virtual ResourceHandle add_resource(const IResource& resource) = 0;

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

    /**
     * @brief High-level object used to render geometry. Renderers can be described as follows:
     * - A Renderer has zero or more @ref IRendererInput. It is planned to allow Renderers without inputs (for example, if vertices are hard-coded within the shader) but this is not yet implemented.
     *      - Drawing multiple inputs is batched as much as possible. Typically, a renderer will emit 1-2 draw calls, regardless of the number of inputs.
     * - A Renderer has exactly one @ref IRendererOutput.
     * - Renderers will never change their input or its corresponding input data.
     * - Renderers cannot have their input/output changed. However, dynamic input data is supported via @ref IRendererDynamicInput.
     * - Renderers have an initial clear-colour but this can be changed after construction. If it is changed during rendering, this may reconstruct the entire render pipeline and incur a large spike in latency.
     * - Renderers will be able to use any number of resources (aka uniform buffers, textures etc...) This is not yet implemented.
     * - Renderers will be able to edit data for a given resource if the resource is writable (e.g SSBOs).
     */
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
        /**
         * @brief Retrieve the number of inputs within the renderer.
         * 
         * @return Number of inputs added to the renderer. Not necessarily the size of the draw-list.
         */
        virtual std::size_t input_count() const = 0;
        /**
         * @brief Retrieve the number of inputs of the given access within the renderer. For example, `input_count_of(tz::gl::RendererInputDataAccess::DynamicFixed)` returns the number of dynamic inputs.
         * 
         * @param access Access specifier of inputs to count.
         * @return Number of inputs added to the renderer with the given access specifier.
         */
        virtual std::size_t input_count_of(RendererInputDataAccess access) const = 0;
        /**
         * @brief Retrieve the renderer input.
         * @note Each renderer takes a copy of the input it was given in its corresponding `IRendererBuilder`. This is NOT the same input as the one you gave to the builder.
         * @note The pointer returned is valid until this Renderer reaches the end of its lifetime.
         * @details For static renderer inputs, you will rarely find this useful. If you have dynamic renderer inputs, you should retrieve a pointer to the input here and edit its data as you wish.
         * 
         * @return IRendererInput* pointing to the renderer's input.
         */
        virtual IRendererInput* get_input(RendererInputHandle handle) = 0;
        /**
         * @brief Retrieve the number of resources associated with the renderer.
         * 
         * @return Number of resources associated with the renderer.
         */
        virtual std::size_t resource_count() const = 0;
        /**
         * @brief Retrieve the number of resources of the given type within the renderer. For exmaple: `resource_count_of(tz::gl::ResourceType::Texture)` returns the number of texture resources.
         * 
         * @param type Resource type of resources to count.
         * @return Number of resources of the given type that are associated with the renderer.
         */
        virtual std::size_t resource_count_of(ResourceType type) const = 0;
        /**
         * @brief Retrieve the resource corresponding to the given handle.
         * 
         * @param handle Handle corresponding to the desired resource.
         * @return Pointer to the resource if such a resource exists within the renderer with this handle. If no such resource exists, nullptr is returned.
         */
        virtual IResource* get_resource(ResourceHandle handle) = 0;

        /**
         * @brief Proceed through the provided render-pass using any inputs and resources.
         * 
         * @note This function does not take in a draw list. This means that it will use the most-recently-provided draw list.
         * If no draw list was ever provided, then a draw list equivalent of once-per-input will be used.
         */
        virtual void render() = 0;
        /**
         * @brief Proceed through the provided render-pass, drawing a list of the inputs.
         * 
         * @param draws List of the input handles to draw in-order. It is valid for the same input to be drawn multiple times. This will also be used for each subsequent render invocation until a new draw list is supplied.
         */
        virtual void render(RendererDrawList draws) = 0;
    };
    /**
     * @}
     */
}

#endif // TOPAZ_GL_API_RENDERER_HPP