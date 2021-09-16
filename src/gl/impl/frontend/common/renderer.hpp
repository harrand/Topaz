#ifndef TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDERER_HPP
#define TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDERER_HPP
#include "gl/api/renderer.hpp"

namespace tz::gl
{
    class RendererBuilderBase : public IRendererBuilder
    {
    public:
        RendererBuilderBase() = default;

        virtual RendererInputHandle add_input(const IRendererInput& input) final;
        virtual const IRendererInput* get_input(RendererInputHandle handle) const final;

        virtual void set_output(IRendererOutput& output) final;
        virtual const IRendererOutput* get_output() const final;
        virtual IRendererOutput* get_output() final;

        virtual ResourceHandle add_resource(const IResource& resource) final;
        virtual const IResource* get_resource(ResourceHandle handle) const final;

        virtual void set_culling_strategy(RendererCullingStrategy culling_strategy) final;
        virtual RendererCullingStrategy get_culling_strategy() const final;
        virtual void set_shader(const Shader& shader) final;
        virtual const Shader& get_shader() const final;
    protected:
        std::vector<const IRendererInput*> inputs;
        IRendererOutput* output = nullptr;
        std::vector<const IResource*> buffer_resources;
        std::vector<const IResource*> texture_resources;
        RendererCullingStrategy culling_strategy = RendererCullingStrategy::NoCulling;
        const Shader* shader = nullptr;
    };
}

#endif // TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDERER_HPP