#ifndef TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#include "gl/api/renderer.hpp"
#include "gl/impl/frontend/common/shader.hpp"

namespace tz::gl2
{
	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Helper struct which the user can use to specify which inputs, resources they want and where they want a Renderer to render to. This is likely going to be refactored at some point because it doesn't do any Vulkan-specific magic.
	 */
	class RendererInfoCommon
	{
	public:
		RendererInfoCommon() = default;
		// Satisfies RendererInfoType.
		/**
		 * Retrieve the number of resources.
		 */
		unsigned int resource_count() const;
		/**
		 * Retrieve the resource corresponding to the given handle.
		 * @param Handle handle returned from a previous call to `add_resource`. If this handle came from a different RendererInfo, the behaviour is undefined.
		 * @return Pointer to the resource.
		 */
		const IResource* get_resource(ResourceHandle handle);
		/**
		 * Retrieve a span containing all of the specified resources. Size of the span is guaranteed to be equal to @ref resource_count()
		 */
		std::span<const IResource* const> get_resources() const;
		/**
		 * Add a new resource, which will be used by a Renderer which is created from this helper struct.
		 *
		 * @param resource Reference to an existing resource. This will be copied by the Renderer upon construction, so its lifetime only needs to last until the desired Renderer has been created.
		 * @return Handle corresponding to the resource. If you want to retrieve the resource later, you should keep ahold of this handle.
		 */
		ResourceHandle add_resource(IResource& resource);
		/**
		 * Renderers always render into something. By default, it renders to the window (only one window is supported so no confusion there). You can however set it to render into something else, such as a @ref TextureOutput if you want to render into the resource of another Renderer.
		 */
		void set_output(IOutput& output);
		/**
		 * Retrieve the current render output (read-only). This may return nullptr, meaning that the main window will be rendered into.
		 */
		const IOutput* get_output() const;
		/**
		 * Retrieve the current render output. This may return nullptr, meaning that the main window will be rendered into.
		 */
		IOutput* get_output();
		/**
		 * Retrieve the currently specified options which will be used by the renderer.
		 */
		const RendererOptions& get_options() const;
		/**
		 * Set the currently specified options which will be used by the renderer.
		 */
		void set_options(RendererOptions options);
		/**
		 * Read/write information about the shader that will be built for the renderer.
		 */
		ShaderInfo& shader();
		/**
		 * Read/write information about the shader that will be built for the renderer.
		 */
		const ShaderInfo& shader() const;
	private:
		/// Stores all provided resources. It is assumed that their lifetime is valid for the entirety of this helper struct's lifetime.
		std::vector<IResource*> resources = {};
		/// Output. Can be null, which defaults to rendering into the main window.
		IOutput* output = nullptr;
		/// Specifies which extra features the Renderer will have.
		RendererOptions options = {};
		/// Describes the shader sources used.
		ShaderInfo shader_info;
	};
	static_assert(RendererInfoType<RendererInfoCommon>);

	template<class Asset>
	class AssetStorageCommon
	{
	public:
		using AssetHandle = tz::Handle<Asset>;
		AssetStorageCommon(std::span<const Asset* const> assets):
		asset_storage()
		{
			for(const Asset* asset : assets)
			{
				this->asset_storage.push_back(asset->unique_clone());
			}
		}
		unsigned int count() const
		{
			return this->asset_storage.size();
		}
		
		const Asset* get(AssetHandle handle) const
		{
			std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
			return this->asset_storage[handle_val].get();
		}

		Asset* get(AssetHandle handle)
		{
			std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
			return this->asset_storage[handle_val].get();
		}
	private:
		std::vector<std::unique_ptr<Asset>> asset_storage;
	};

}

#endif // TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
