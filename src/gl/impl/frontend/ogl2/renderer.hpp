#ifndef TOPAZ_GL2_IMPL_FRONTEND_OGL2_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_OGL2_RENDERER_HPP
#if TZ_OGL
#include "gl/api/renderer.hpp"
#include "gl/api/component.hpp"
#include "gl/impl/frontend/common/renderer.hpp"
#include "gl/impl/backend/ogl2/vertex_array.hpp"
#include "gl/impl/backend/ogl2/shader.hpp"

namespace tz::gl2
{
	using namespace tz::gl;

	class ResourceStorage : public AssetStorageCommon<IResource>
	{
	public:
		ResourceStorage(std::span<const IResource* const> resources);
		const IComponent* get_component(ResourceHandle handle) const;
		IComponent* get_component(ResourceHandle handle);
	private:
		std::vector<std::unique_ptr<IComponent>> components;
	};

	class ShaderManager
	{
	public:
		ShaderManager(const ShaderInfo& sinfo);
		void use();
	private:
		ogl2::Shader make_shader(const ShaderInfo& sinfo) const;

		ogl2::Shader shader;
	};

	using RendererInfoOGL = RendererInfoCommon;

	class RendererOGL
	{
	public:
		RendererOGL(const RendererInfoOGL& info);
		// Satisfies RendererType.
		unsigned int resource_count() const;
		const IResource* get_resource(ResourceHandle handle) const;
		IResource* get_resource(ResourceHandle handle);
		const IComponent* get_component(ResourceHandle handle) const;
		IComponent* get_component(ResourceHandle handle);
		const RendererOptions& get_options() const;

		void render();
		void render(unsigned int tri_count);
	private:
		ogl2::VertexArray vao;
		ResourceStorage resources;
		ShaderManager shader;
		RendererOptions options;
		unsigned int tri_count = 0;
	};
	static_assert(RendererType<RendererOGL>);
}

#endif // TZ_OGL
#endif // TOPAZ_GL2_IMPL_FRONTEND_OGL2_RENDERER_HPP
