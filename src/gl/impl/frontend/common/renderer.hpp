#ifndef TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
#include "core/memory.hpp"
#include "gl/api/renderer.hpp"
#include "gl/impl/frontend/common/shader.hpp"

namespace tz::gl
{
	/**
	 * @ingroup tz_gl2_renderer
	 * Helper class which can be used to generate a @ref RendererEditRequest.
	 */
	class RendererEditBuilder
	{
	public:
		RendererEditBuilder() = default;
		/**
		 * Make amendments to the compute configuration of the renderer. See @ref RendererComputeEditRequest for details.
		 */
		RendererEditBuilder& compute(RendererComputeEditRequest req);
		/**
		 * Make amendments to the current render state. See @ref RendererStateEditRequest for details.
		 */
		RendererEditBuilder& render_state(RendererStateEditRequest req);
		/**
		 * Make amendments to an existing image resource. See @ref RendererImageComponentEditRequest for details.
		 */
		RendererEditBuilder& image(RendererImageComponentEditRequest req);
		/**
		 * Make amendments to an existing buffer resource. See @ref RendererBufferComponentEditRequest for details.
		 */
		RendererEditBuilder& buffer(RendererBufferComponentEditRequest req);

		/**
		 * Retrieve a @ref RendererEditRequest corresponding to all edits specified within the builder so far.
		 */
		RendererEditRequest build() const;
	private:
		RendererEditRequest request = {.component_edits = {}};
	};
	/**
	 * @ingroup tz_gl2_renderer
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
		std::span<const IComponent* const> get_components() const;
		/**
		 * Add a new resource, which will be used by a Renderer which is created from this helper struct.
		 *
		 * @param resource Reference to an existing resource. This will be copied by the Renderer upon construction, so its lifetime only needs to last until the desired Renderer has been created.
		 * @return Handle corresponding to the resource. If you want to retrieve the resource later, you should keep ahold of this handle.
		 */
		ResourceHandle add_resource(IResource& resource);
		ResourceHandle add_component(IComponent& component);
		/**
		 * Renderers always render into something. By default, it renders to the window (only one window is supported so no confusion there). You can however set it to render into something else, such as a @ref TextureOutput if you want to render into the resource of another Renderer.
		 */
		void set_output(const IOutput& output);
		/**
		 * Retrieve the current render output (read-only). This may return nullptr, meaning that the main window will be rendered into.
		 */
		const IOutput* get_output() const;
		/**
		 * Retrieve the currently specified options which will be used by the renderer.
		 */
		const RendererOptions& get_options() const;
		/**
		 * Set the currently specified options which will be used by the renderer.
		 */
		void set_options(RendererOptions options);
		/**
		 * Set the clear colour value. Default is solid black. {0, 0, 0, 1}
		 */
		void set_clear_colour(tz::Vec4 clear_colour);
		/**
		 * Retrieve the current clear colour value.
		 */
		const tz::Vec4& get_clear_colour() const;
		/**
		 * Set the compute kernel (aka workgroup dimensions) for any compute work dispatched by this renderer. If the renderer does not do any compute work, this is unused.
		 */
		void set_compute_kernel(tz::Vec3ui compute_kernel);
		/**
		 * Retrieve the compute kernel (aka workgroup dimensions) for compute work. By default, this is {1, 1, 1}.
		 */
		const tz::Vec3ui& get_compute_kernel() const;
		/**
		 * Read/write information about the shader that will be built for the renderer.
		 */
		ShaderInfo& shader();
		/**
		 * Read/write information about the shader that will be built for the renderer.
		 */
		const ShaderInfo& shader() const;
		std::string debug_get_name() const;
	private:
		std::size_t real_resource_count() const;
		/// Stores all provided resources. It is assumed that their lifetime is valid for the entirety of this helper struct's lifetime.
		std::vector<IResource*> resources = {};
		/// Stores all provided components. In this context, components act as references to existing resources owned by another renderer.
		std::vector<IComponent*> components = {};
		/// Output. Can be null, which defaults to rendering into the main window.
		std::unique_ptr<IOutput> output = nullptr;
		/// Specifies which extra features the Renderer will have.
		RendererOptions options = {};
		/// Describes the shader sources used.
		ShaderInfo shader_info;
		/// The clear value for colour attachments.
		tz::Vec4 clear_colour = {0.0f, 0.0f, 0.0f, 1.0f};
		/// Workgroup sizes if we're doing compute.
		tz::Vec3ui compute_kernel = {1u, 1u, 1u};
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
				if(asset == nullptr)
				{
					this->asset_storage.push_back(nullptr);
				}
				else
				{
					this->asset_storage.push_back(asset->unique_clone());
				}
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

		void set(AssetHandle handle, Asset* value)
		{
			std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
			tz_assert(!this->asset_storage[handle_val].owning(), "AssetStorageCommon: Try to set specific asset value, but the asset at that handle is not a reference (it is owned by us)");
			this->asset_storage[handle_val] = value;
		}
	private:
		std::vector<MaybeOwnedPtr<Asset>> asset_storage;
	};

}

#endif // TOPAZ_GL2_IMPL_FRONTEND_COMMON_RENDERER_HPP
