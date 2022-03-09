#ifndef TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP
#if TZ_VULKAN
#include "core/window_functionality.hpp"
#include "gl/api/renderer.hpp"
#include "gl/api/component.hpp"
#include "gl/impl/frontend/common/renderer.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"
#include "gl/impl/backend/vk2/image.hpp"
#include "gl/impl/backend/vk2/image_view.hpp"
#include "gl/impl/backend/vk2/render_pass.hpp"
#include "gl/impl/backend/vk2/framebuffer.hpp"
#include "gl/impl/backend/vk2/sampler.hpp"
#include "gl/impl/backend/vk2/descriptors.hpp"
#include "gl/impl/backend/vk2/pipeline_layout.hpp"
#include "gl/impl/backend/vk2/graphics_pipeline.hpp"
#include "gl/impl/backend/vk2/hardware/queue.hpp"
#include "gl/impl/backend/vk2/command.hpp"
#include "gl/impl/backend/vk2/semaphore.hpp"
#include "gl/impl/backend/vk2/fence.hpp"
#include "gl/impl/backend/vk2/swapchain.hpp"

namespace tz::gl
{
	using namespace tz::gl;
	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend
	 * @defgroup tz_gl2_graphicsapi_vk_frontend_renderer Renderer Implementation
	 * Documentation for the Vulkan Frontend implementation of @ref RendererType.
	 */

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Copies all resource data from a RendererVulkan when it is created, and copies it into its own memory so that the user doesn't need to worry about resource lifetimes. Also exposes the copied resources to the RendererVulkan.
	 */
	class ResourceStorage : public AssetStorageCommon<IResource>
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
		/**
		 * Retrieve the descriptor layout representing the shader resources used by the renderer.
		 */
		const vk2::DescriptorLayout& get_descriptor_layout() const;
		/**
		 * Retrieve a view into the array of descriptor sets.
		 */
		std::span<const vk2::DescriptorSet> get_descriptor_sets() const;

		std::size_t resource_count_of(ResourceType type) const;
		void sync_descriptors(bool write_everything);
		bool empty() const;
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
		 * @param create_depth_images Whether we should create depth images or not. If so, they will also be passed into the framebuffer. This means that the graphics pipeline the renderer ends up using will also need to know that we're using a depth attachment.
		 * @param ldev Vulkan LogicalDevice which will be used to construct the render-pass and framebuffers etc. Right now we expect this to be the exact same LogicalDevice everywhere throughout this RendererVulkan. However this may change in the future (albeit unlikely tbh).
		 */
		OutputManager(const IOutput* output, std::span<vk2::Image> window_buffer_images, bool create_depth_images, const vk2::LogicalDevice& ldev);
		/**
		 * Retrieve the render pass used by the renderer.
		 */
		const vk2::RenderPass& get_render_pass() const;
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
		bool has_depth_images() const;
		void create_output_resources(std::span<vk2::Image> window_buffer_images, bool create_depth_images);
	private:
		/// Output provided by the RendererVulkan.
		const IOutput* output;
		/// Logical device used to create depth images, render passes and framebuffers.
		const vk2::LogicalDevice* ldev;
		/// List of window buffer images (offscreen image or swapchain images) from the Device.
		std::span<vk2::Image> window_buffer_images;
		/// List of depth images for each window buffer image (These may be null if depth testing is disabled).
		std::vector<vk2::Image> window_buffer_depth_images;
		/// List of image-views, one for each output image. These haven't been re-ordered in any way FYI.
		std::vector<vk2::ImageView> output_imageviews;
		/// List of depth-image-views, one for each output image.
		std::vector<vk2::ImageView> output_depth_imageviews;
		/// We don't support multiple sub-passes, so this is a run-of-the-mill basic-bitch render pass.
		vk2::RenderPass render_pass;
		/// List of framebuffers, one for each output image. These also haven't been re-ordered in any way FYI.
		std::vector<vk2::Framebuffer> output_framebuffers;
	};

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Retrieves the output and resource storage state and creates a shader and graphics pipeline to be used to render everything. If changes occur in the output/resource state then this is responsible for reflecting such changes in the pipeline if necessary.
	 */
	class GraphicsPipelineManager
	{
	public:
		/**
		 * Construct the pipeline manager using all the necessary shader sources aswell as resource state and output information.
		 * @param sinfo Information about the shader which will be used.
		 * @param dlayout Describes the shader resource format used by the renderer.
		 * @param frame_in_flight_count Describes the number of frames we have in flight. The descriptor layout will need to copy the format this amount of times.
		 * @param viewport_dimensions Dimensions of the viewport, in pixels. Output images associated with the render pass may need to match these dimensions.
		 * @param depth_testing_enabled Specifies whether we want to create a graphics pipeline which will perform depth tests or not. If the output manager was told to create depth images, this should be enabled (otherwise the framebuffer will not match the provided render pass).
		 */
		GraphicsPipelineManager(const ShaderInfo& sinfo, const vk2::DescriptorLayout& dlayout, const vk2::RenderPass& render_pass, std::size_t frame_in_flight_count, tz::Vec2ui viewport_dimensions, bool depth_testing_enabled, bool alpha_blending_enabled);

		/**
		 * Retrieve the vulkan graphics pipeline which will be used for rendering.
		 * @return Graphics pipeline which should be bound when rendering.
		 */
		const vk2::GraphicsPipeline& get_pipeline() const;
		void recreate(const vk2::RenderPass& new_render_pass, tz::Vec2ui new_viewport_dimensions);
	private:
		vk2::Shader make_shader(const vk2::LogicalDevice& ldev, const ShaderInfo& sinfo) const;
		vk2::PipelineLayout make_pipeline_layout(const vk2::DescriptorLayout& dlayout, std::size_t frame_in_flight_count) const;

		vk2::Shader shader;
		vk2::PipelineLayout pipeline_layout;
		vk2::GraphicsPipeline graphics_pipeline;
		bool depth_testing_enabled;
	};

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Responsible for all required command-buffers and drawing/scratch commands. The Renderer should not need to worry about command pools/buffers at all, but instead ask the command processor to schedule work and it can do so.
	 */
	class CommandProcessor
	{
	public:
		struct RenderWorkSubmitResult
		{
			vk2::hardware::Queue::PresentResult present;
		};
		/**
		 * Construct a command processor, which will render into the provided output framebuffers. A command buffer will be created for each frame-in-flight, aswell as an extra buffer for scratch commands.
		 * @param ldev LogicalDevice used to create command state.
		 * @param frame_in_flight_count Number of frames to be doing work on in parallel.
		 * @param output_target Describes what it is we are actually trying to render into, i.e a Window of offscreen Image.
		 * @param output_framebuffers Array of framebuffers belonging to the output manager which will act as our render targets. The array should have length equal to `frame_in_flight_count`.
		 * @pre `output_framebuffers.size() == frame_in_flight_count`, otherwise the behaviour is undefined.
		 */
		CommandProcessor(vk2::LogicalDevice& ldev, std::size_t frame_in_flight_count, OutputTarget output_target, std::span<vk2::Framebuffer> output_framebuffers);
		/**
		 * Retrieve a list of all command buffers which will be used for rendering. Each command buffer is guaranteed to have the exact same commands recorded within them.
		 * @return Array of command buffers, length matching the number of frames-in-flight.
		 */
		std::span<const vk2::CommandBuffer> get_render_command_buffers() const;
		/**
		 * Retrieve a list of all command buffers which will be used for rendering. Each command buffer is guaranteed to have the exact same commands recorded within them.
		 * @return Array of command buffers, length matching the number of frames-in-flight.
		 */
		std::span<vk2::CommandBuffer> get_render_command_buffers();
		/**
		 * Record all vulkan commands invoked in the provided function into the scratch command buffer, and submit it instantly. This function is synchronous so is guaranteed to return once the work is all done.
		 * @param record_commands Function which will be invoked with the scratch buffer recording. The function should use the provided recording to invoke vulkan commands.
		 */
		void do_scratch_operations(tz::Action<vk2::CommandBufferRecording&> auto record_commands);
		/**
		 * Record all vulkan commands invoked in the provided function into each of the rendering command buffers. The index of the current rendering command buffer is also passed to the provided function.
		 * @param record_commands Function which will be invoked with a recording for each render command buffer, aswell as the index of the command buffer being recorded.
		 */
		void set_rendering_commands(tz::Action<vk2::CommandBufferRecording&, std::size_t> auto record_commands);
		/**
		 * Submit the next render command buffer. If the output is presentable, it will also be presented. This function is asynchronous so you should not expect the submit/present work to be done upon return.
		 * @param maybe_swapchain Pointer to the swapchain which will be presented to. If the renderer is not expected to perform any presentation, this can be nullptr.
		 * @return Structure containing information about the results of the render work submission (and image presentation if applicable).
		 * @pre `maybe_swapchain` must not be nullptr if the renderer is expected to present the result of the submitted work to the window.
		 * @note Headless rendering is not yet implemented.
		 */
		RenderWorkSubmitResult do_render_work(vk2::Swapchain* maybe_swapchain);
		void wait_pending_commands_complete();
	private:
		/// Stores whether we expect to present submitted results to a swapchain.
		bool requires_present;
		/// Hardware queue which will be used for the render work submission (and presentation, if we need to do so).
		vk2::hardware::Queue* graphics_queue;
		/// Pool which handles allocation of command buffers.
		vk2::CommandPool command_pool;
		/// Stores allocated command buffers.
		vk2::CommandPool::AllocationResult commands;
		/// Stores the number of frames we expect to have in flight.
		std::size_t frame_in_flight_count;
		/// List of semaphores, one for each frame in flight. Represents when a swapchain image is available.
		std::vector<vk2::BinarySemaphore> image_semaphores;
		/// List of semaphores, one for each frame in flight. Represents when submitted render work for a frame has been completed (does not include presentation).
		std::vector<vk2::BinarySemaphore> render_work_semaphores;
		/// List of fences, one for each frame in flight. Represents when all work for a given frame (submission + presentation) has completed and the frame is considered 'fully complete'. 
		std::vector<vk2::Fence> in_flight_fences;
		/// Helper list which refers to each in-flight-fence, but in an order useful to the swapchain image acquisition logic.
		std::vector<const vk2::Fence*> images_in_flight;
		/// The image index most recently acquired from the swapchain.
		std::uint32_t output_image_index = 0;
		/// Represents the current index of the frame (between 0 and frame_in_flight_count - 1).
		std::size_t current_frame = 0;

	};

	using RendererInfoVulkan = RendererInfoCommon;


	struct RendererResizeInfoVulkan
	{
		tz::Vec2ui new_dimensions;
		std::span<vk2::Image> new_output_images;
	};

	using RendererResizeCallbackType = tz::Callback<RendererResizeInfoVulkan>;

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
		/// Callback for resizing which the renderer subscribes to for the duration of its lifetime. Assume this isn't null but for headless applications this should not be used.
		RendererResizeCallbackType* resize_callback;
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
		 * @param info User-exposed class which describes how many resources etc. we have and a high-level description of where we expect to render into.
		 * @param device_info A renderer is always created by a Device - This constructor is not invoked manually. When the Device does this, it provides some information about the internals; this.
		 */
		RendererVulkan(const RendererInfoVulkan& info, const RendererDeviceInfoVulkan& device_info);
		~RendererVulkan();
		// Satisfies RendererType
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
		 * Retrieve options denoting extra features used by the renderer.
		 */
		const RendererOptions& get_options() const;
		/**
		 * Invoke the renderer, emitting a single draw call of a set number of triangles. The number of triangles renderered is equal to the number of triangles rendered in the previous draw-call. If this is the first draw, zero triangles are rendered.
		 */
		void render();
		/**
		 * Invoke the renderer, emitting a single draw call of a set number of triangles.
		 * @param tri_count Number of triangles to render.
		 */
		void render(unsigned int tri_count);
		/**
		 * Confirm changes to a renderer.
		 *
		 * Editing renderers is expensive, so it should only be done if absolutely necessary. If you are editing renderers on a per-frame basis, consider creating multiple different renderers upfront for each hot-path and switching between them as necessary instead.
		 * @param edit_request Structure specifying which edits to make.
		 */
		void edit(const RendererEditRequest& edit_request);
	private:
		void setup_static_resources();
		void setup_render_commands();
		void handle_resize(const RendererResizeInfoVulkan& resize_info);

		vk2::LogicalDevice* ldev;
		/// Stores copies of all provided resources, and deals with all the vulkan descriptor magic. Exposes everything relevant to us when we want to draw.
		ResourceStorage resources;
		/// Handles output image component logic, and exposes a nice list of images/views/framebuffers into which we can render into without having to worry about the complicated logic behind the output wrangling.
		OutputManager output;
		GraphicsPipelineManager pipeline;
		CommandProcessor command;
		vk2::Swapchain* maybe_swapchain;
		RendererOptions options;
		tz::Vec4 clear_colour;
		unsigned int tri_count = 0;
		RendererResizeCallbackType* device_resize_callback = nullptr;
		tz::CallbackHandle window_resize_callback = tz::nullhand;
	};

	static_assert(RendererType<RendererVulkan>);
}

#include "gl/impl/frontend/vk2/renderer.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP
