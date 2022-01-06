#ifndef TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP
#if TZ_VULKAN
#include "gl/2/api/renderer.hpp"
#include "gl/2/api/component.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"
#include "gl/impl/backend/vk2/image.hpp"
#include "gl/impl/backend/vk2/image_view.hpp"
#include "gl/impl/backend/vk2/render_pass.hpp"
#include "gl/impl/backend/vk2/framebuffer.hpp"
#include "gl/impl/backend/vk2/sampler.hpp"
#include "gl/impl/backend/vk2/descriptors.hpp"

namespace tz::gl2
{
	using namespace tz::gl;
	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend
	 * @defgroup tz_gl2_graphicsapi_vk_frontend_renderer Renderer Implementation
	 * Documentation for the Vulkan Frontend implementation of @ref RendererType.
	 */

	template<class Asset>
	class AssetStorage
	{
	public:
		using AssetHandle = tz::Handle<Asset>;
		AssetStorage(std::span<const Asset* const> assets):
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

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Copies all input data from a RendererVulkan when it is created, and copies it all into its own memory so that the user doesn't need to worry about input lifetimes. Also exposes these copied inputs to the RendererVulkan.
	 */
	using InputStorage = AssetStorage<IInput>;

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Copies all resource data from a RendererVulkan when it is created, and copies it into its own memory so that the user doesn't need to worry about resource lifetimes. Also exposes the copied resources to the RendererVulkan.
	 */
	class ResourceStorage : public AssetStorage<IResource>
	{
	public:
		/**
		 * Create storage for a set of existing resources.
		 *
		 * All existing ResourceHandles referencing any of the provided resources will continue to be valid when passed to the RendererVulkan. However, it will reference the copy of said resource which is created during construction. This means users are able and encouraged to cache their ResourceHandles when populating RendererInfoVulkan.
		 *
		 * @param resources A view into an array of existing resources. All of these will be copied into separate store, meaning the lifetime of the elements of the span need not last beyond this constructor's execution.
		 * @param ldev Vulkan LogicalDevice, this will be used to handle the various components and vulkan descriptor shenanigans.
		 */
		ResourceStorage(std::span<const IResource* const> resources, const vk2::LogicalDevice& ldev);
		/**
		 * Retrieve the component (read-only) which stores the corresponding vulkan backend objects for the resource corresponding to the handle.
		 * @param handle Handle whose resource's component needs to be retrieved. The handle must have referred to one of the initial resources passed to the constructor, otherwise the behaviour is undefined.
		 */
		const IComponent* get_component(ResourceHandle handle) const;
		/**
		 * Retrieve the component which stores the corresponding vulkan backend objects for the resource corresponding to the handle.
		 * @param handle Handle whose resource's component needs to be retrieved. The handle must have referred to one of the initial resources passed to the constructor, otherwise the behaviour is undefined.
		 */
		IComponent* get_component(ResourceHandle handle);
	private:
		/// Storage for all cloned resource's components.
		std::vector<std::unique_ptr<IComponent>> components;
		/// An ImageView for each ImageResource that was passed to the constructor. These are views referring to the corresponding ImageComponent to said resource.
		std::vector<vk2::ImageView> image_component_views;
		/// Hard-coded sampler info. This might need to be editable in the future, but for now the user has no control over this. Care must be taken to ensure that other graphics API frontends sample images in the same way.
		vk2::Sampler basic_sampler;
		/// Vulkan Descriptor Set layout, which matches the layout of the provided buffer and image resources. Note that buffer resources get their own binding, but all image resources are a single descriptor array.
		vk2::DescriptorLayout descriptor_layout;
		/// Storage for DescriptorSets.
		vk2::DescriptorPool descriptor_pool;
		/// Stores the above pool's allocation result. We know the exact number of descriptors/sets etc that we need, so we only ever need a single allocation for now.
		vk2::DescriptorPool::AllocationResult descriptors;
	};

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Deciphering the actual render target is pretty complicated. There are various gotchas:
	 * - The tz::ApplicationType might be Headless, meaning that we don't have a vk2::Swapchain at all, and the Device instead exposed some offscreen images.
	 * - If there is a swapchain, we don't have control over how many swapchain images are available. We need to be flexible as the Device has sole control over this.
	 * - If we're rendering into a separate image (most likely a texture resource belonging to another RendererVulkan), that RendererVulkan has ownership of the image component, not us.
	 * - If we're rendering to the window, the Device used to create this RendererVulkan has ownership of those image components. This is true if there is a swapchain available, and even if we're a headless application and the Device has still provided us with an offscreen image.
	 *
	 * This class deals with all of those cases and exposes a list of output images, aswell as imageviews and framebuffers for them.
	 */
	class OutputManager
	{
	public:
		/**
		 * Construct the manager to deal with this brain-knot of output components.
		 * @param output Output which is either going to be a WindowOutput (containing swapchain images or a headless render-target image) or a TextureOutput which owns its own TextureComponent which we will need to extract.
		 * @param window_buffer_images View into the array of WindowOutput images. These all belong to the creator Device. If we're a headless application, this is likely going to be a single offscreen render-target image. Otherwise, it's going to be the swapchain images. If the output is a TextureOutput then we're not going to use any of these.
		 * @param ldev Vulkan LogicalDevice which will be used to construct the render-pass and framebuffers etc. Right now we expect this to be the exact same LogicalDevice everywhere throughout this RendererVulkan. However this may change in the future (albeit unlikely tbh).
		 */
		OutputManager(IOutput* output, std::span<vk2::Image> window_buffer_images, const vk2::LogicalDevice& ldev);
		/**
		 * Retrieve a view into all of the output images.
		 *
		 * There is no way to guarantee how many output images there are, but here is an explanation of what you might get:
		 * - If the output is a window output:
		 * 	- If Topaz was initialised headlessly, this is most likely going to be a single offscreen image which was provided by the Device.
		 *  	- If Topaz is running in a window, this is going to contain the swapchain images. Unfortunately this may vary depending on the machine running and what it supports, but in practice most likely this will be a couple (2-4).
		 * @return List of output images.
		 */
		std::span<vk2::Image> get_output_images();
		/**
		 * Retrieve a view (read-only) into all of the framebuffers.
		 *
		 * Note that a framebuffer is guaranteed to only have a single image view, because we do not yet support multiple-render-targets. This means that the size of this span is guaranteed to be equal to the number of output images (see @ref get_output_images()).
		 * @return List of framebuffers for each output image.
		 */
		std::span<const vk2::Framebuffer> get_output_framebuffers() const;
		/**
		 * Retrieve a view into all of the framebuffers.
		 *
		 * Note that a framebuffer is guaranteed to only have a single image view, because we do not yet support multiple-render-targets. This means that the size of this span is guaranteed to be equal to the number of output images (see @ref get_output_images()).
		 * @return List of framebuffers for each output image.
		 */
		std::span<vk2::Framebuffer> get_output_framebuffers();
		/**
		 * Retrieve the dimensions of the output images. The output images are guaranteed to all be the same size.
		 * @return {width, height} of the output images, in pixels.
		 */
		tz::Vec2ui get_output_dimensions() const;
	private:
		/// Output provided by the RendererVulkan.
		IOutput* output;
		/// List of window buffer images (offscreen image or swapchain images) from the Device.
		std::span<vk2::Image> window_buffer_images;
		/// List of image-views, one for each output image. These haven't been re-ordered in any way FYI.
		std::vector<vk2::ImageView> output_imageviews;
		/// We don't support multiple sub-passes, so this is a run-of-the-mill basic-bitch render pass.
		vk2::RenderPass render_pass;
		/// List of framebuffers, one for each output image. These also haven't been re-ordered in any way FYI.
		std::vector<vk2::Framebuffer> output_framebuffers;
	};

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Helper struct which the user can use to specify which inputs, resources they want and where they want a Renderer to render to. This is likely going to be refactored at some point because it doesn't do any Vulkan-specific magic.
	 */
	class RendererInfoVulkan
	{
	public:
		RendererInfoVulkan() = default;
		// Satisfies RendererInfoType.
		/**
		 * Retrieve the number of inputs.
		 */
		unsigned int input_count() const;
		/**
		 * Retrieve the input corresponding to the given handle.
		 * @param Handle handle returned from a previous call to `add_input`. If this handle came from a different RendererInfo, the behaviour is undefined.
		 * @return Pointer to the input.
		 */
		const IInput* get_input(InputHandle handle);
		/**
		 * Retrieve a span containing all of the specified inputs. Size of the span is guaranteed to be equal to @ref input_count()
		 */
		std::span<const IInput* const> get_inputs() const;
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
		 * Add a new input, which will be used by a Renderer which is created from this helper struct.
		 *
		 * @param input Reference to an existing input. This will be copied by the Renderer upon construction, so its lifetime only needs to last until the desired Renderer has been created.
		 * @return Handle corresponding to the input. If you want to retrieve the input later, you should keep ahold of this handle.
		 */
		InputHandle add_input(IInput& input);
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
	private:
		/// Stores all provided inputs. It is assumed that their lifetime is valid for the entirety of this helper struct's lifetime.
		std::vector<IInput*> inputs = {};
		/// Stores all provided resources. It is assumed that their lifetime is valid for the entirety of this helper struct's lifetime.
		std::vector<IResource*> resources = {};
		/// Output. Can be null, which defaults to rendering into the main window.
		IOutput* output = nullptr;
	};
	static_assert(RendererInfoType<RendererInfoVulkan>);

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * When the Device wants to create a RendererVulkan, we need to know a little bit about the Device's internals (such as the swapchain images, or an offscreen image if we're headless).
	 */
	struct RendererDeviceInfoVulkan
	{
		/// LogicalDevice used to create VKAPI objects. Most likely comes from a DeviceVulkan.
		vk2::LogicalDevice* device;
		/// List of output images. If the output is a window, this is likely to be swapchain images or an offscreen headless image.
		std::span<vk2::Image> output_images;
		/// Swapchain if there is one.
		vk2::Swapchain* maybe_swapchain;
	};

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Renderer implementation which heavily calls into the backend at @ref tz_gl_vk.
	 */
	class RendererVulkan
	{
	public:
		/// Hard-coded. We process frames in parallel on the GPU. This is the maximum number of frames we're working on at a time.
		static constexpr std::size_t max_frames_in_flight = 2;
		/**
		 * Create a new Renderer.
		 * @param info User-exposed class which describes how many inputs, resources etc. we have and a high-level description of where we expect to render into.
		 * @param device_info A renderer is always created by a Device - This constructor is not invoked manually. When the Device does this, it provides some information about the internals; this.
		 */
		RendererVulkan(RendererInfoVulkan& info, const RendererDeviceInfoVulkan& device_info);
		// Satisfies RendererType
		/**
		 * Retrieve the number of inputs.
		 */
		unsigned int input_count() const;
		/**
		 * Retrieve the cloned input (read-only) corresponding to the given handle.
		 * @param Handle handle returned from a call to a RendererInfoVulkan's `add_input`. If this handle came from a RendererInfoVulkan different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the input.
		 */
		const IInput* get_input(InputHandle handle) const;
		/**
		 * Retrieve the cloned input corresponding to the given handle.
		 * @param Handle handle returned from a call to a RendererInfoVulkan's `add_input`. If this handle came from a RendererInfoVulkan different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the input.
		 */
		IInput* get_input(InputHandle handle);
		/**
		 * Retrieve the number of resources.
		 */
		unsigned int resource_count() const;
		/**
		 * Retrieve the resource (read-only) corresponding to the given handle.
		 * @param Handle handle returned from a call to a RendererInfoVulkan's `add_resource`. If this handle came from a RendererInfoVulkan different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource.
		 */
		const IResource* get_resource(ResourceHandle handle) const;
		/**
		 * Retrieve the resource corresponding to the given handle.
		 * @param Handle handle returned from a call to a RendererInfoVulkan's `add_resource`. If this handle came from a RendererInfoVulkan different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource.
		 */
		IResource* get_resource(ResourceHandle handle);
		/**
		 * Retrieve the component sourcing the resource (read-only) corresponding to the given handle.
		 * @param Handle handle returned from a call to a RendererInfoVulkan's `add_resource`. If this handle came from a RendererInfoVulkan different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource's underlying component.
		 */
		const IComponent* get_component(ResourceHandle handle) const;
		/**
		 * Retrieve the component sourcing the resource corresponding to the given handle.
		 * @param Handle handle returned from a call to a RendererInfoVulkan's `add_resource`. If this handle came from a RendererInfoVulkan different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource's underlying component.
		 */
		IComponent* get_component(ResourceHandle handle);
		/**
		 * Render all inputs that were activated by the most recently-provided draw-list. If no draw-list was ever provided, this will render every input. Note: This is not true yet lmao - it does fuck all.
		 */
		void render(){}
	private:
		/// Stores copies of all provided inputs and exposes them in a neater way.
		InputStorage inputs;
		/// Stores copies of all provided resources, and deals with all the vulkan descriptor magic. Exposes everything relevant to us when we want to draw.
		ResourceStorage resources;
		/// Handles output image component logic, and exposes a nice list of images/views/framebuffers into which we can render into without having to worry about the complicated logic behind the output wrangling.
		OutputManager output;
	};

	static_assert(RendererType<RendererVulkan>);
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP
