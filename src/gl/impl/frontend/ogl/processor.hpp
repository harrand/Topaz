#ifndef TZ_GL_IMPL_FRONTEND_OGL_PROCESSOR_HPP
#define TZ_GL_IMPL_FRONTEND_OGL_PROCESSOR_HPP
#if TZ_OGL
#include "gl/api/processor.hpp"
#include "gl/impl/frontend/ogl/component.hpp"
#include <vector>

namespace tz::gl
{
    class ProcessorBuilderOGL : public IProcessorBuilder
    {
    public:
        ProcessorBuilderOGL() = default;
        virtual ResourceHandle add_resource(const IResource& resource) final;
        virtual const IResource* get_resource(ResourceHandle handle) const final;
        virtual void set_shader(const Shader& shader) final;
        virtual const Shader& get_shader() const final;

        std::span<const IResource* const> ogl_get_buffer_resources() const;
        std::span<const IResource* const> ogl_get_texture_resources() const;
    private:
        std::vector<const IResource*> buffer_resources;
        std::vector<const IResource*> texture_resources;
        const Shader* shader = nullptr;
    };

    class ProcessorOGL : public IProcessor
    {
    public:
        ProcessorOGL(ProcessorBuilderOGL builder);
        virtual std::size_t resource_count() const final;
        virtual std::size_t resource_count_of(ResourceType type) const final;
        virtual IResource* get_resource(ResourceHandle handle) final;
        virtual void process() final;
    private:
        void setup_buffers();
        void setup_textures();

        std::vector<std::unique_ptr<IResource>> buffer_resources;
        std::vector<std::unique_ptr<IResource>> texture_resources;
        std::vector<BufferComponentOGL> buffer_components;
        std::vector<TextureComponentOGL> texture_components;
        const Shader* shader;
    };
}

#endif // TZ_OGL
#endif // TZ_GL_IMPL_FRONTEND_OGL_PROCESSOR_HPP