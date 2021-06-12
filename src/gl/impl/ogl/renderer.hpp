#ifndef TOPAZ_GL_IMPL_OGL_RENDERER_HPP
#define TOPAZ_GL_IMPL_OGL_RENDERER_HPP
#include "gl/api/renderer.hpp"

#include "glad/glad.h"
#include <optional>

namespace tz::gl
{
    class RendererBuilderOGL : public IRendererBuilder
    {
    public:
        RendererBuilderOGL() = default;
        virtual RendererInputHandle add_input(const IRendererInput& input) final;
        virtual const IRendererInput* get_input(RendererInputHandle handle) const final;

        virtual void set_output(const IRendererOutput& output) final;
        virtual const IRendererOutput* get_output() const final;

        virtual ResourceHandle add_resource(const IResource& resource) final;

        virtual void set_culling_strategy(RendererCullingStrategy culling_strategy) final;
        virtual RendererCullingStrategy get_culling_strategy() const final;
        
        virtual void set_render_pass(const RenderPass& render_pass) final;
        virtual const RenderPass& get_render_pass() const final;
        
        virtual void set_shader(const Shader& shader) final;
        virtual const Shader& get_shader() const final;

        std::span<const IResource* const> ogl_get_buffer_resources() const;
        std::span<const IResource* const> ogl_get_texture_resources() const;
        std::span<const IRendererInput* const> ogl_get_inputs() const;
    private:
        std::vector<const IRendererInput*> inputs;
        const IRendererOutput* output = nullptr;
        std::vector<const IResource*> buffer_resources;
        std::vector<const IResource*> texture_resources;
        const RenderPass* render_pass = nullptr;
        const Shader* shader = nullptr;
        RendererCullingStrategy culling_strategy;
    };

    class RendererOGL : public IRenderer
    {
    public:
        RendererOGL(RendererBuilderOGL builder);
        RendererOGL(const RendererOGL& copy) = delete;
        RendererOGL(RendererOGL&& move);
        ~RendererOGL();

        RendererOGL& operator=(const RendererOGL& rhs) = delete;
        RendererOGL& operator=(RendererOGL&& rhs);

        virtual void set_clear_colour(tz::Vec4 clear_colour) final;
        virtual tz::Vec4 get_clear_colour() const final;
        virtual IRendererInput* get_input(RendererInputHandle handle) final;
        virtual IResource* get_resource(ResourceHandle handle) final;
        
        virtual void render() final;
    private:
        std::vector<std::unique_ptr<IRendererInput>> copy_inputs(const RendererBuilderOGL& builder);
        std::vector<IRendererInput*> get_inputs();
        std::size_t num_static_inputs() const;
        std::size_t num_dynamic_inputs() const;

        GLuint vao;
        GLuint vbo, ibo, vbo_dynamic, ibo_dynamic;
        GLuint indirect_buffer, indirect_buffer_dynamic;
        std::vector<std::unique_ptr<IResource>> resources;
        std::vector<GLuint> resource_ubos;
        std::vector<GLuint> resource_textures;
        RendererElementFormat format;
        const RenderPass* render_pass;
        const Shader* shader;
        std::vector<std::unique_ptr<IRendererInput>> inputs;
        const IRendererOutput* output;
    };
}

#endif // TOPAZ_GL_IMPL_OGL_RENDERER_HPP