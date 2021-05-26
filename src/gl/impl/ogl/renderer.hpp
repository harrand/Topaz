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
        virtual void set_input(const IRendererInput& input) final;
        virtual const IRendererInput* get_input() const final;

        virtual void set_output(const IRendererOutput& output) final;
        virtual const IRendererOutput* get_output() const final;

        virtual void add_resource(unsigned int resource_id, const IResource& resource) final;
        virtual void remove_resource(unsigned int resource_id) final;
        virtual const IResource* get_resource(unsigned int resource_id) const final;

        virtual void set_culling_strategy(RendererCullingStrategy culling_strategy) final;
        virtual RendererCullingStrategy get_culling_strategy() const final;

        virtual void set_render_pass(const RenderPass& render_pass) final;
        virtual const RenderPass& get_render_pass() const final;

        virtual void set_shader(const Shader& shader) final;
        virtual const Shader& get_shader() const final;
    private:
        const IRendererInput* input = nullptr;
        const IRendererOutput* output = nullptr;
        const RenderPass* render_pass = nullptr;
        const Shader* shader = nullptr;
        std::optional<RendererElementFormat> format;
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
        virtual IRendererInput* get_input() final;
        
        virtual void render() final;
    private:
        GLuint vao;
        GLuint vbo, ibo;
        GLsizei index_count;
        const RenderPass* render_pass;
        const Shader* shader;
        std::unique_ptr<IRendererInput> input;
        const IRendererOutput* output;
    };
}

#endif // TOPAZ_GL_IMPL_OGL_RENDERER_HPP