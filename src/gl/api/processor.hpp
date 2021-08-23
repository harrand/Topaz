#ifndef TOPAZ_GL_API_PROCESSOR_HPP
#define TOPAZ_GL_API_PROCESSOR_HPP
#include "gl/api/resource.hpp"
#include "gl/shader.hpp"

namespace tz::gl
{
    /**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * A collection of low-level renderer-agnostic graphical interfaces.
	 * @{
	 */

    /**
     * @brief Structure describing the nature of a processor.     * 
     */
    class IProcessorBuilder
    {
    public:
        /**
         * @brief Add a resource to the processor, meaning that its data may be used during computation.
         * @note When a Processor is created, any resources referred to within its builder are copied, meaning that the resources need not exist beyond that point for that particular processor.
         * @param resource Reference to an existing resource. The IResource must refer to a valid object until the desired Processor has been created.
         * @return ResourceHandle Handle which can be used to refer to the resource provided. The ResourceHandle is also valid for any Renderer created from this builder.
         */
        virtual ResourceHandle add_resource(const IResource& resource) = 0;
        /**
         * @brief Retrieve a resource using the given handle.
         * @pre `handle` must refer to an existing resource that was earlier registered via @ref IProcessorBuilder::add_resource.
         * 
         * @param handle Handle whose corresponding resource should be retrieved.
         * @return const IResource* pointing to the resource.
         */
        virtual const IResource* get_resource(ResourceHandle handle) const = 0;
        /**
         * @brief Set the shader which will be used during computation. This must contain exactly one compute-shader.
         * This is non-optional. All processors *must* have a shader.
         * 
         * @param shader Existing Shader program. Unlike resources/inputs, the shader must exist throughout the lifetime of the builder, and any spawned Processors.
         */
        virtual void set_shader(const Shader& shader) = 0;
        /**
         * @brief Retrieve the shader that will be used during computation.
         * @pre @ref IProcessorBuilder::set_shader must have been invoked earlier, so a shader is currently being referred to.
         * @return const Shader& Reference to an existing Shader program.
         */
        virtual const Shader& get_shader() const = 0;
    };

    /**
     * @brief High-level object used for general-purpose GPU computation via a compute shader. Processors can be described by the following key points:
     * - A Processor has no inputs or outputs, unlike a Renderer
     * - A Processor contains zero or more resources and a shader. The shader may or may not use the resources.
     * - A Processor always has write-access to its resources.
     */
    class IProcessor
    {
    public:
        /**
         * @brief Retrieve the number of resources associated with the Processor.
         * 
         * @return Number of resources associated with the Processor.
         */
        virtual std::size_t resource_count() const = 0;
        /**
         * @brief Retrieve the number of resources of the given type within the Processor. For exmaple: `resource_count_of(tz::gl::ResourceType::Texture)` returns the number of texture resources.
         * 
         * @param type Resource type of resources to count.
         * @return Number of resources of the given type that are associated with the Processor.
         */
        virtual std::size_t resource_count_of(ResourceType type) const = 0;
        /**
         * @brief Retrieve the resource corresponding to the given handle.
         * 
         * @param handle Handle corresponding to the desired resource.
         * @return Pointer to the resource if such a resource exists within the Processor with this handle. If no such resource exists, nullptr is returned.
         */
        virtual IResource* get_resource(ResourceHandle handle) = 0;
        /**
         * @brief Invoke the Processor, executing its corresponding compute-shader using all provided resources.
         */
        virtual void process() = 0;
    };

    /**
     * @}
     */
}

#endif // TOPAZ_GL_API_PROCESSOR_HPP