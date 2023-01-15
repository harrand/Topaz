#ifndef TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP
#if TZ_VULKAN
#include "tz/gl/api/renderer.hpp"
#include "tz/gl/api/component.hpp"
#include "tz/gl/impl/common/renderer.hpp"
#include "tz/gl/impl/vulkan/detail/logical_device.hpp"
#include "tz/gl/impl/vulkan/detail/image.hpp"
#include "tz/gl/impl/vulkan/detail/image_view.hpp"
#include "tz/gl/impl/vulkan/detail/render_pass.hpp"
#include "tz/gl/impl/vulkan/detail/framebuffer.hpp"
#include "tz/gl/impl/vulkan/detail/sampler.hpp"
#include "tz/gl/impl/vulkan/detail/descriptors.hpp"
#include "tz/gl/impl/vulkan/detail/pipeline_layout.hpp"
#include "tz/gl/impl/vulkan/detail/graphics_pipeline.hpp"
#include "tz/gl/impl/vulkan/detail/hardware/queue.hpp"
#include "tz/gl/impl/vulkan/detail/command.hpp"
#include "tz/gl/impl/vulkan/detail/semaphore.hpp"
#include "tz/gl/impl/vulkan/detail/fence.hpp"
#include "tz/gl/impl/vulkan/detail/swapchain.hpp"
#include "tz/core/callback.hpp"

namespace tz::gl
{
	class DeviceWindowVulkan;
	class DeviceRenderSchedulerVulkan;
	using namespace tz::gl;
	using RendererInfoVulkan = RendererInfoCommon;

	struct RendererResizeInfoVulkan
	{
		hdk::vec2ui new_dimensions;
		std::span<vk2::Image> new_output_images;
		vk2::Image* new_depth_image;
	};

	using RendererResizeCallbackType = tz::Callback<RendererResizeInfoVulkan>;

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
		 */
		ResourceStorage(const RendererInfoVulkan& info);
		ResourceStorage() = default;
		ResourceStorage(ResourceStorage&& move);
		~ResourceStorage() = default;
		ResourceStorage& operator=(ResourceStorage&& rhs);
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
		/**
		 * Notifies that an ImageComponent at the provided handle has had its underlying vk2::Image re-seated. This recreates any necessary image views.
		 */
		void notify_image_recreated(tz::gl::ResourceHandle image_resource_handle);
		/**
		 * Updates all relevant descriptors. If buffers are resized for example, their underlying vk2::Buffer has been re-seated and thus the descriptors need to be re-synced.
		 * @param write_everything If true, image component views are also written. This should only need to happen during renderer construction or if an image component renderer edit is being performed.
		 */
		void sync_descriptors(bool write_everything, const RenderState& state);
		/**
		 * Query as to whether there are any resources or not.
		 * @return True if there is at least one buffer or image resource, otherwise false. This includes resource references.
		 */
		bool empty() const;
		/**
		 * Query as to whether there are any shader resources or not.
		 * Some buffer resources are not descriptor-related, such as index and draw indirect buffers. If a renderer for example has a single index buffer, it is not empty() but descriptor_empty().
		 * @return True if there are no descriptor-related resources.
		 */
		bool descriptor_empty() const;
		/**
		 * This must happen every frame. For each dynamic image resource, pad the current data and update the mapped data. This is because vk2::Image mappings have an implementation-defined row padding, which this function is responsible for correcting.
		 */
		void write_padded_image_data();
	private:

		/// Storage for all cloned resource's components.
		std::vector<tz::MaybeOwnedPtr<IComponent>> components = {};
		/// An ImageView for each ImageResource that was passed to the constructor. These are views referring to the corresponding ImageComponent to said resource.
		std::vector<vk2::ImageView> image_component_views = {};
		// A unique sampler for every single image. There is no duplicate checking, so there may be redundant samplers in here. However, it's not trivial to fix this because we use combined image sampling - to use separate image and samplers requires shader source changes, which means big tzslc changes for vulkan only. Looks like it could end up changing syntax so we avoid this for the time being.
		std::vector<vk2::Sampler> samplers = {};
		/// Vulkan Descriptor Set layout, which matches the layout of the provided buffer and image resources. Note that buffer resources get their own binding, but all image resources are a single descriptor array.
		vk2::DescriptorLayout descriptor_layout = vk2::DescriptorLayout::null();
		/// Storage for DescriptorSets.
		vk2::DescriptorPool descriptor_pool = vk2::DescriptorPool::null();
		/// Stores the above pool's allocation result. We know the exact number of descriptors/sets etc that we need, so we only ever need a single allocation for now.
		vk2::DescriptorPool::AllocationResult descriptors = {};
		/// Describes the number of frames that are in-flight at once.
		std::size_t frame_in_flight_count = 0;
	};

	/**
	 * Represents the image state of a renderer output.
	 */
	struct OutputImageState
	{
		tz::BasicList<vk2::Image*> colour_attachments = {};
		vk2::Image* depth_attachment = nullptr;
	};

	/**
	 * Represent the imageview state of a renderer output.
	 */
	struct OutputImageViewState
	{
		tz::BasicList<vk2::ImageView> colour_views = {};
	};

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Deciphering the actual render target is pretty complicated. There are various gotchas:
	 * - If there is a swapchain, we don't have control over how many swapchain images are available. We need to be flexible as the Device has sole control over this.
	 * - If we're rendering into a separate image (most likely a texture resource belonging to another RendererVulkan), that RendererVulkan has ownership of the image component, not us.
	 * - If we're rendering to the window, the Device used to create this RendererVulkan has ownership of those image components. This should always be the case.
	 *
	 * This class deals with all of those cases and exposes a list of output images, aswell as imageviews and framebuffers for them.
	 */
	class OutputManager
	{
	public:
		/**
		 * Construct the manager to deal with this brain-knot of output components.
		 */
		OutputManager(const RendererInfoVulkan& info);
		OutputManager() = default;
		OutputManager(OutputManager&& move);
		~OutputManager() = default;
		OutputManager& operator=(OutputManager&& rhs);
		/**
		 * Retrieve the render pass used by the renderer.
		 */
		const vk2::RenderPass& get_render_pass() const;
		/**
		 * Retrieve a view into all of the output images.
		 *
		 * There is no way to guarantee how many output images there are, but here is an explanation of what you might get:
		 * - If the output is a window output:
		 * 		- This is going to contain the swapchain images. Unfortunately this may vary depending on the machine running and what it supports, but in practice most likely this will be a couple (2-4).
		 * - Otherwise if the output is an ImageOutput:
		 *   		- This is going to contain the 0th colour attachment, with duplicates matching each frame-in-flight.
		 * @return List of output images.
		 */
		std::vector<OutputImageState> get_output_images();
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
		hdk::vec2ui get_output_dimensions() const;

		IOutput* get_output();
		const IOutput* get_output() const;

		bool has_depth_images() const;
		void create_output_resources(std::span<vk2::Image> swapchain_images, vk2::Image* depth_image);
	private:
		void populate_output_views();
		void make_render_pass();
		void populate_framebuffers();
		/// Output provided by the RendererVulkan.
		std::unique_ptr<IOutput> output = nullptr;
		/// Logical device used to create depth images, render passes and framebuffers.
		const vk2::LogicalDevice* ldev = nullptr;
		/// List of window buffer images (offscreen image or swapchain images) from the Device.
		std::span<vk2::Image> swapchain_images = {};
		/// List of depth images for each window buffer image (These may be null if depth testing is disabled).
		vk2::Image* swapchain_depth_images = {};
		tz::gl::RendererOptions options = {};
		/// List of image-views, one for each output image. These haven't been re-ordered in any way FYI.
		std::vector<OutputImageViewState> output_imageviews = {};
		/// List of depth-image-views, one for each output image.
		std::vector<vk2::ImageView> output_depth_imageviews = {};
		/// We don't support multiple sub-passes, so this is a run-of-the-mill basic-bitch render pass.
		vk2::RenderPass render_pass = vk2::RenderPass::null();
		/// List of framebuffers, one for each output image. These also haven't been re-ordered in any way FYI.
		std::vector<vk2::Framebuffer> output_framebuffers = {};
	};

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Retrieves the output and resource storage state and creates a shader and graphics pipeline to be used to render everything. If changes occur in the output/resource state then this is responsible for reflecting such changes in the pipeline if necessary.
	 */
	class GraphicsPipelineManager
	{
	public:
		/**
		 * Construct the graphics/compute pipeline manager using all the necessary shader sources aswell as resource state and output information.
		 */
		GraphicsPipelineManager(const RendererInfoVulkan& info, const ResourceStorage& resources, const OutputManager& output);
		GraphicsPipelineManager() = default;
		GraphicsPipelineManager(GraphicsPipelineManager&& move);
		~GraphicsPipelineManager() = default;
		GraphicsPipelineManager& operator=(GraphicsPipelineManager&& rhs);

		/**
		 * Retrieve the vulkan graphics pipeline which will be used for rendering.
		 * @return Graphics pipeline which should be bound when rendering.
		 */
		const vk2::Pipeline& get_pipeline() const;
		const vk2::Shader& get_shader() const;
		vk2::Shader& get_shader();
		void recreate(const vk2::RenderPass& new_render_pass, hdk::vec2ui new_viewport_dimensions, bool wireframe_mode = false);
		bool is_compute() const;
		bool is_wireframe_mode() const;
	private:
		vk2::Shader make_shader(const vk2::LogicalDevice& ldev, const ShaderInfo& sinfo) const;
		vk2::PipelineLayout make_pipeline_layout(const vk2::DescriptorLayout& dlayout, std::size_t frame_in_flight_count) const;
		vk2::GraphicsPipelineInfo make_graphics_pipeline(hdk::vec2ui viewport_dimensions, bool depth_testing_enabled, bool alpha_blending_enabled, const vk2::RenderPass& render_pass) const;
		vk2::ComputePipelineInfo make_compute_pipeline() const;
		vk2::Pipeline make_pipeline(hdk::vec2ui viewport_dimensions, bool depth_testing_enabled, bool alpha_blending_enabled, const vk2::RenderPass& render_pass) const;

		vk2::Shader shader = vk2::Shader::null();
		vk2::PipelineLayout pipeline_layout = vk2::PipelineLayout::null();
		vk2::Pipeline graphics_pipeline = vk2::Pipeline::null();
		bool depth_testing_enabled = false;
		bool wireframe_mode = false;
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
		 */
		CommandProcessor(const RendererInfoVulkan& info);
		CommandProcessor(vk2::LogicalDevice& ldev, std::size_t frame_in_flight_count, OutputTarget output_target, std::span<vk2::Framebuffer> output_framebuffers, bool instant_compute_enabled, tz::gl::RendererOptions options, DeviceRenderSchedulerVulkan& scheduler);
		CommandProcessor() = default;
		CommandProcessor(const CommandProcessor& copy) = delete;
		CommandProcessor(CommandProcessor&& move);
		~CommandProcessor() = default;
		CommandProcessor& operator=(const CommandProcessor& rhs) = delete;
		CommandProcessor& operator=(CommandProcessor&& rhs);
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
		 */
		RenderWorkSubmitResult do_render_work();
		void do_compute_work();

		void wait_pending_commands_complete();
	private:
		/// Stores whether we expect to present submitted results to a swapchain.
		bool requires_present = false;
		bool instant_compute_enabled = false;
		/// Hardware queue which will be used for the render work submission (and presentation, if we need to do so).
		vk2::hardware::Queue* graphics_queue = nullptr;
		/// Hardware queue which will be used for any compute work that needs to be done.
		vk2::hardware::Queue* compute_queue = nullptr;
		/// Pool which handles allocation of command buffers.
		vk2::CommandPool command_pool = vk2::CommandPool::null();
		/// Stores allocated command buffers.
		vk2::CommandPool::AllocationResult commands = {};
		/// Stores the number of frames we expect to have in flight.
		std::size_t frame_in_flight_count = 0;
		/// Helper list which refers to each in-flight-fence, but in an order useful to the swapchain image acquisition logic.
		std::vector<const vk2::Fence*> images_in_flight = {};
		/// Copy of all renderer options specified.
		tz::gl::RendererOptions options = {};
		/// Scheduler object belonging to the parent device. We retrieve scheduling primitives from here, such as semaphores and fences.
		DeviceRenderSchedulerVulkan* device_scheduler = nullptr;
		/// The image index most recently acquired from the swapchain.
		std::uint32_t output_image_index = 0;
		/// Represents the current index of the frame (between 0 and frame_in_flight_count - 1).
		std::size_t current_frame = 0;

	};

	/**
	 * @ingroup tz_gl2_graphicsapi_vk_frontend_renderer
	 * Renderer implementation which heavily calls into the backend at @ref tz_gl_vk.
	 */
	class RendererVulkan
	{
	public:
		/**
		 * Create a new Renderer.
		 * @param info User-exposed class which describes how many resources etc. we have and a high-level description of where we expect to render into.
		 * @param device_info A renderer is always created by a Device - This constructor is not invoked manually. When the Device does this, it provides some information about the internals; this.
		 */
		RendererVulkan(const RendererInfoVulkan& info);
		RendererVulkan(RendererVulkan&& move);
		~RendererVulkan();
		RendererVulkan& operator=(RendererVulkan&& rhs);
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
		IOutput* get_output();
		const IOutput* get_output() const;
		/**
		 * Retrieve options denoting extra features used by the renderer.
		 */
		const RendererOptions& get_options() const;
		const RenderState& get_state() const;
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
		void dbgui();
		std::string_view debug_get_name() const;

		static RendererVulkan null();
		bool is_null() const;
	private:
		RendererVulkan() = default;

		struct EditData
		{
			constexpr static int descriptor_resync_partial = 0x01;
			constexpr static int descriptor_resync_full = 0x11;
			int descriptor_resync = 0;
			bool pipeline_recreate = false;
			bool commands_rerecord = false;
			bool static_resources_rewrite = false;
			bool operator==(const EditData& rhs) const = default;
		};

		void edit_buffer_resize(RendererEdit::BufferResize arg, EditData& data);
		void edit_image_resize(RendererEdit::ImageResize arg, EditData& data);
		void edit_resource_write(RendererEdit::ResourceWrite arg, EditData& data);
		void edit_compute_config(RendererEdit::ComputeConfig arg, EditData& data);
		void edit_resource_reference(RendererEdit::ResourceReference arg, EditData& data);

		void setup_static_resources();
		void setup_render_commands();
		void setup_compute_commands();
		void setup_work_commands();
		bool handle_swapchain_outdated();
		void handle_resize(const RendererResizeInfoVulkan& resize_info);

		std::size_t present_failure_count = 0;
		// LogicalDevice that every vulkan backend object will use.
		vk2::LogicalDevice* ldev = nullptr;
		// Contains which renderer options were enabled.
		RendererOptions options = {};
		// Current state of the renderer.
		RenderState state = {};
		/// Stores copies of all provided resources, and deals with all the vulkan descriptor magic. Exposes everything relevant to us when we want to draw.
		ResourceStorage resources = {};
		/// Handles output image component logic, and exposes a nice list of images/views/framebuffers into which we can render into without having to worry about the complicated logic behind the output wrangling.
		OutputManager output = {};
		/// Helper object for managing the underlying graphics/compute pipeline.
		GraphicsPipelineManager pipeline = {};
		/// Helper object for managing/executing/scheduling GPU work.
		CommandProcessor command = {};
		std::string debug_name = "Null Renderer";
		tz::gl::ScissorRegion scissor_cache = tz::gl::ScissorRegion::null();
		hdk::vec2ui window_dims_cache = {};
	};

	static_assert(RendererType<RendererVulkan>);
}

#include "tz/gl/impl/vulkan/renderer.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP