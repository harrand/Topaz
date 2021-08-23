#ifndef TOPAZ_GL_IMPL_FRONTEND_VK_PROCESSOR_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK_PROCESSOR_HPP
#if TZ_VULKAN
#include "gl/api/processor.hpp"

namespace tz::gl
{
    class ProcessorBuilderVulkan : public IProcessorBuilder
    {
    public:
        ProcessorBuilderVulkan() = default;
        virtual ResourceHandle add_resource(const IResource& resource) final;
        virtual const IResource* get_resource(ResourceHandle handle) const final;
        virtual void set_shader(const Shader& shader) final;
        virtual const Shader& get_shader() const final;
    private:
        std::vector<const IResource*> buffer_resources;
        std::vector<const IResource*> texture_resources;
        const Shader* shader = nullptr;
    };

    class ProcessorVulkan
    {
    public:

    private:

    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK_PROCESSOR_HPP