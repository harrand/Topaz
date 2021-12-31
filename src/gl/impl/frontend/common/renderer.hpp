#ifndef TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDERER_HPP
#define TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDERER_HPP
#include "gl/api/renderer.hpp"
#include <memory>
#include <vector>

namespace tz::gl
{
	class RendererBuilderBase : public IRendererBuilder
	{
	public:
		RendererBuilderBase() = default;

		virtual RendererInputHandle add_input(const IRendererInput& input) override;
		virtual const IRendererInput* get_input(RendererInputHandle handle) const override;
		virtual std::size_t input_count() const override;

		virtual void set_output(IRendererOutput& output) override;
		virtual const IRendererOutput* get_output() const override;
		virtual IRendererOutput* get_output() override;

		virtual ResourceHandle add_resource(const IResource& resource) override;
		virtual const IResource* get_resource(ResourceHandle handle) const override;
		virtual std::span<const IResource* const> get_resources(ResourceType type) const override;

		virtual void set_culling_strategy(RendererCullingStrategy culling_strategy) override;
		virtual RendererCullingStrategy get_culling_strategy() const override;
		virtual void set_shader(const IShader& shader) override;
		virtual const IShader& get_shader() const override;
	protected:
		std::vector<const IRendererInput*> inputs;
		IRendererOutput* output = nullptr;
		std::vector<const IResource*> buffer_resources;
		std::vector<const IResource*> texture_resources;
		RendererCullingStrategy culling_strategy = RendererCullingStrategy::NoCulling;
		const IShader* shader = nullptr;
	};

	class RendererBase : public IRenderer
	{
	public:
		RendererBase(const IRendererBuilder& builder);
		RendererBase() = default;

		// IRenderer
		virtual void set_clear_colour(tz::Vec4 clear_colour) override;
		virtual tz::Vec4 get_clear_colour() const override;

		virtual std::size_t input_count() const override;
		virtual std::size_t input_count_of(RendererInputDataAccess access) const override;
		virtual IRendererInput* get_input(RendererInputHandle handle) override;
		virtual std::size_t resource_count() const override;
		virtual std::size_t resource_count_of(ResourceType type) const override;
		virtual IResource* get_resource(ResourceHandle handle) override;
	protected:
		static std::vector<std::unique_ptr<IRendererInput>> copy_inputs(const IRendererBuilder& builder);
		static std::vector<std::unique_ptr<IResource>> copy_resources(ResourceType type, const IRendererBuilder& builder);

		std::vector<std::unique_ptr<IRendererInput>> inputs;
		std::vector<std::unique_ptr<IResource>> buffer_resources;
		std::vector<std::unique_ptr<IResource>> texture_resources;
		tz::Vec4 clear_colour;
	};
}

#endif // TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDERER_HPP
