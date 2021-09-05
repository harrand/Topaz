#ifndef TOPAZ_GL_IMPL_FRONTEND_COMMON_PROCESSOR_HPP
#define TOPAZ_GL_IMPL_FRONTEND_COMMON_PROCESSOR_HPP
#include "gl/api/processor.hpp"
#include <vector>

namespace tz::gl
{
    class ProcessorBuilderBase : public IProcessorBuilder
    {
    public:
        ProcessorBuilderBase() = default;
        virtual ResourceHandle add_resource(const IResource& resource) override;
        virtual const IResource* get_resource(ResourceHandle handle) const override;
        virtual void set_shader(const Shader& shader) override;
        virtual const Shader& get_shader() const override;
    protected:
        std::vector<const IResource*> buffer_resources;
        std::vector<const IResource*> texture_resources;
        const Shader* shader = nullptr;
    };
}

#endif // TOPAZ_GL_IMPL_FRONTEND_COMMON_PROCESSOR_HPP