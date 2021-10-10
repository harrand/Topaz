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
	using RendererBuilderOGL = RendererBuilderBase;

	struct RendererDeviceInfoOGL
	{
		DeviceWindowResizeCallback* on_resize;
	};

	class RendererOGL : public RendererBase
	{
	public:
		RendererOGL(RendererBuilderOGL builder, RendererDeviceInfoOGL device_info);
		RendererOGL(const RendererOGL& copy) = delete;
		RendererOGL(RendererOGL&& move);
		~RendererOGL();

		RendererOGL& operator=(const RendererOGL& rhs) = delete;
		RendererOGL& operator=(RendererOGL&& rhs);

		virtual void set_clear_colour(tz::Vec4 clear_colour) final;
		virtual IComponent* get_component(ResourceHandle handle) final;

		virtual void render() final;
		virtual void render(RendererDrawList draw_list) final;
	private:
		void setup_output_framebuffer();
		void bind_draw_list(const RendererDrawList& list);
		bool draws_match_cache(const RendererDrawList& list) const;
		RendererDrawList all_inputs_once() const;
		std::size_t num_static_inputs() const;
		std::size_t num_dynamic_inputs() const;
		std::size_t num_static_draws() const;
		std::size_t num_dynamic_draws() const;
		void resize_output_component();
		void handle_resize();

		GLuint vao;
		std::optional<ogl::Buffer> vbo, ibo, vbo_dynamic, ibo_dynamic;
		std::optional<ogl::Buffer> indirect_buffer, indirect_buffer_dynamic;
		std::vector<BufferComponentOGL> resource_buffers;
		std::vector<TextureComponentOGL> resource_textures;
		RendererElementFormat format;
		const Shader* shader;
		const IRendererOutput* output;
		TextureComponentOGL* output_texture_component;
		std::optional<ogl::Framebuffer> output_framebuffer;
		std::optional<ogl::Renderbuffer> output_depth_renderbuffer;
		RendererDrawList draw_cache;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_OGL_RENDERER_HPP