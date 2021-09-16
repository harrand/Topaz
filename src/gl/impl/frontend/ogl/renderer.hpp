#ifndef TOPAZ_GL_IMPL_OGL_RENDERER_HPP
#define TOPAZ_GL_IMPL_OGL_RENDERER_HPP
#if TZ_OGL
#include "gl/api/renderer.hpp"
#include "gl/declare/device.hpp"
#include "gl/impl/frontend/common/renderer.hpp"
#include "gl/impl/frontend/ogl/component.hpp"
#include "gl/impl/backend/ogl/buffer.hpp"
#include "gl/impl/backend/ogl/framebuffer.hpp"

#include <optional>

namespace tz::gl
{
    class RendererBuilderOGL : public RendererBuilderBase
    {
    public:
        RendererBuilderOGL() = default;
        std::span<const IResource* const> ogl_get_buffer_resources() const;
        std::span<const IResource* const> ogl_get_texture_resources() const;
        std::span<const IRendererInput* const> ogl_get_inputs() const;
    };

    struct RendererDeviceInfoOGL
    {
        DeviceWindowResizeCallback* on_resize;
    };

    class RendererOGL : public IRenderer
    {
    public:
        RendererOGL(RendererBuilderOGL builder, RendererDeviceInfoOGL device_info);
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
        void setup_output_framebuffer();
        void bind_draw_list(const RendererDrawList& list);
        bool draws_match_cache(const RendererDrawList& list) const;
        RendererDrawList all_inputs_once() const;
        std::vector<std::unique_ptr<IRendererInput>> copy_inputs(const RendererBuilderOGL& builder);
        std::vector<IRendererInput*> get_inputs();
        std::size_t num_static_inputs() const;
        std::size_t num_dynamic_inputs() const;
        std::size_t num_static_draws() const;
        std::size_t num_dynamic_draws() const;
        void resize_output_component();
        void handle_resize();

        GLuint vao;
        std::optional<ogl::Buffer> vbo, ibo, vbo_dynamic, ibo_dynamic;
        //GLuint vbo, ibo, vbo_dynamic, ibo_dynamic;
        std::optional<ogl::Buffer> indirect_buffer, indirect_buffer_dynamic;
        //GLuint indirect_buffer, indirect_buffer_dynamic;
        std::vector<std::unique_ptr<IResource>> resources;
        std::vector<BufferComponentOGL> resource_buffers;
        std::vector<TextureComponentOGL> resource_textures;
        RendererElementFormat format;
        const Shader* shader;
        std::vector<std::unique_ptr<IRendererInput>> inputs;
        const IRendererOutput* output;
        TextureComponentOGL* output_texture_component;
        std::optional<ogl::Framebuffer> output_framebuffer;
        std::optional<ogl::Renderbuffer> output_depth_renderbuffer;
        RendererDrawList draw_cache;
    };
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_OGL_RENDERER_HPP