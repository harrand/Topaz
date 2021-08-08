#ifndef TOPAZ_GL_IMPL_OGL_RENDERER_HPP
#define TOPAZ_GL_IMPL_OGL_RENDERER_HPP
#if TZ_OGL
#include "gl/api/renderer.hpp"
#include "gl/impl/frontend/ogl/component.hpp"
#include "gl/impl/backend/ogl/buffer.hpp"

#include <optional>

namespace tz::gl
{
    class RendererBuilderOGL : public IRendererBuilder
    {
    public:
        RendererBuilderOGL() = default;
        virtual RendererInputHandle add_input(const IRendererInput& input) final;
        virtual const IRendererInput* get_input(RendererInputHandle handle) const final;

        virtual void set_pass(RenderPassAttachment pass) final;
        virtual RenderPassAttachment get_pass() const final;

        virtual void set_output(IRendererOutput& output) final;
        virtual const IRendererOutput* get_output() const final;
        virtual IRendererOutput* get_output() final;

        virtual ResourceHandle add_resource(const IResource& resource) final;

        virtual void set_culling_strategy(RendererCullingStrategy culling_strategy) final;
        virtual RendererCullingStrategy get_culling_strategy() const final;
        
        virtual void set_shader(const Shader& shader) final;
        virtual const Shader& get_shader() const final;

        std::span<const IResource* const> ogl_get_buffer_resources() const;
        std::span<const IResource* const> ogl_get_texture_resources() const;
        std::span<const IRendererInput* const> ogl_get_inputs() const;
    private:
        std::vector<const IRendererInput*> inputs;
        RenderPassAttachment pass = RenderPassAttachment::ColourDepth;
        IRendererOutput* output = nullptr;
        std::vector<const IResource*> buffer_resources;
        std::vector<const IResource*> texture_resources;
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

        virtual std::size_t input_count() const final;
        virtual std::size_t input_count_of(RendererInputDataAccess access) const final;

        virtual IRendererInput* get_input(RendererInputHandle handle) final;

        virtual std::size_t resource_count() const final;
        virtual std::size_t resource_count_of(ResourceType type) const final;
        virtual IResource* get_resource(ResourceHandle handle) final;
        virtual IComponent* get_component(ResourceHandle handle) final;

        virtual void render() final;
        virtual void render(RendererDrawList draw_list) final;
    private:
        void bind_draw_list(const RendererDrawList& list);
        bool draws_match_cache(const RendererDrawList& list) const;
        RendererDrawList all_inputs_once() const;
        std::vector<std::unique_ptr<IRendererInput>> copy_inputs(const RendererBuilderOGL& builder);
        std::vector<IRendererInput*> get_inputs();
        std::size_t num_static_inputs() const;
        std::size_t num_dynamic_inputs() const;
        std::size_t num_static_draws() const;
        std::size_t num_dynamic_draws() const;

        GLuint vao;
        std::optional<ogl::Buffer> vbo, ibo, vbo_dynamic, ibo_dynamic;
        //GLuint vbo, ibo, vbo_dynamic, ibo_dynamic;
        std::optional<ogl::Buffer> indirect_buffer, indirect_buffer_dynamic;
        //GLuint indirect_buffer, indirect_buffer_dynamic;
        std::vector<std::unique_ptr<IResource>> resources;
        std::vector<GLuint> resource_ubos;
        std::vector<TextureComponentOGL> resource_textures;
        RendererElementFormat format;
        RenderPassAttachment pass_attachment;
        const Shader* shader;
        std::vector<std::unique_ptr<IRendererInput>> inputs;
        const IRendererOutput* output;
        TextureComponentOGL* output_texture_component;
        RendererDrawList draw_cache;
    };
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_OGL_RENDERER_HPP