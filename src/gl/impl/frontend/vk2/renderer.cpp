#if TZ_VULKAN
#include "gl/declare/image_format.hpp"
#include "gl/impl/backend/vk2/fixed_function.hpp"
#include "gl/impl/backend/vk2/gpu_mem.hpp"
#include "gl/impl/backend/vk2/descriptors.hpp"
#include "gl/impl/backend/vk2/image_view.hpp"
#include "gl/impl/frontend/vk2/renderer.hpp"
#include "gl/impl/frontend/vk2/component.hpp"
#include "gl/output.hpp"

namespace tz::gl2
{
	using namespace tz::gl;

//--------------------------------------------------------------------------------------------------
	ResourceStorage::ResourceStorage(std::span<const IResource* const> resources, const vk2::LogicalDevice& ldev):
	AssetStorageCommon<IResource>(resources),
	components(),
	image_component_views(),
	basic_sampler(vk2::SamplerInfo
	{
		.device = &ldev,
		.min_filter = vk2::LookupFilter::Nearest,
		.mag_filter = vk2::LookupFilter::Nearest,
		.mipmap_mode = vk2::MipLookupFilter::Nearest,
		.address_mode_u = vk2::SamplerAddressMode::ClampToEdge,
		.address_mode_v = vk2::SamplerAddressMode::ClampToEdge,
		.address_mode_w = vk2::SamplerAddressMode::ClampToEdge,
	}),
	descriptor_layout(vk2::DescriptorLayout::null()),
	descriptor_pool(vk2::DescriptorPool::null()),
	descriptors()
	{
		std::vector<bool> buffer_id_to_variable_access;
		for(std::size_t i = 0; i < this->count(); i++)
		{
			IResource* res = this->get(static_cast<tz::HandleValue>(i));
			switch(res->get_type())
			{
				case ResourceType::Buffer:
					this->components.push_back(std::make_unique<BufferComponentVulkan>(*res, ldev));
					if(res->get_access() == ResourceAccess::DynamicFixed || res->get_access() == ResourceAccess::DynamicVariable)
					{
						// Tell the resource to use the buffer's data. Also copy whatever we had before.
						std::span<const std::byte> initial_data = res->data();
						std::span<std::byte> buffer_byte_data = static_cast<BufferComponentVulkan*>(this->components.back().get())->vk_get_buffer().map_as<std::byte>();
						std::copy(initial_data.begin(), initial_data.end(), buffer_byte_data.begin());
						res->set_mapped_data(buffer_byte_data);
					}
					buffer_id_to_variable_access.push_back(res->get_access() == ResourceAccess::DynamicVariable);
				break;
				case ResourceType::Image:
					this->components.push_back(std::make_unique<ImageComponentVulkan>(*res, ldev));
					// If we're an image, we need to create an image view too.
					{
						vk2::Image& result_image = static_cast<ImageComponentVulkan*>(this->components.back().get())->vk_get_image();
						this->image_component_views.emplace_back
							(vk2::ImageViewInfo{
								.image = &result_image,
								.aspect = vk2::ImageAspect::Colour
							 });
					}
				break;
				default:
					tz_error("Unrecognised ResourceType. Please submit a bug report.");
				break;
			}
		}

		// Figure out Descriptor stuffs. We are going to use bindless here.
		// Firstly get all the buffer resources into one list of pointers. We already have a list of imageviews from earlier we can use.
		//std::vector<vk2::Buffer*> buffers;
		//for(const auto& component_ptr : this->components)
		//{
		//	if(component_ptr->get_resource()->get_type() != ResourceType::Buffer)
		//	{
		//		continue;
		//	}
		//	buffers.push_back(&static_cast<BufferComponentVulkan*>(component_ptr.get())->vk_get_buffer());
		//}
		std::size_t buffer_count = this->resource_count_of(ResourceType::Buffer);
		// So buffer_id_to_variable_access stores a bool for each buffer (in-order). True if the access is variable, false otherwise. However, if any buffer at all is variable, we will unfortunately try to write to them all in sync_descriptors.
		// Until we fix sync_descriptors to be a little less cavalier, we must add the descriptor-indexing flags and take the perf loss if there are *any* variable-sized buffer resources at all.
		{
			vk2::DescriptorLayoutBuilder lbuilder;
			lbuilder.set_device(ldev);
			// Each buffer gets their own binding id.
			for(std::size_t i = 0; i < buffer_count; i++)
			{
				// TODO: Only add the necessary flags to the buffers with variable access instead of all of them if any have it. Dependent on changes to sync_descriptors.
				vk2::DescriptorFlags desc_flags;
				if(std::any_of(buffer_id_to_variable_access.begin(), buffer_id_to_variable_access.end(),
				[](bool b){return b;}))
				{
					desc_flags = 
					{
						vk2::DescriptorFlag::UpdateAfterBind,
						vk2::DescriptorFlag::UpdateUnusedWhilePending
					};
				}

				lbuilder.with_binding
				({
					.type = vk2::DescriptorType::StorageBuffer,
					.count = 1,
					.flags = desc_flags
				});
			}
			// And one giant descriptor array for all textures. Wow.
			lbuilder.with_binding
			({
				.type = vk2::DescriptorType::ImageWithSampler,
				.count = static_cast<std::uint32_t>(this->image_component_views.size()),
				.flags = {vk2::DescriptorFlag::PartiallyBound}
			});
			this->descriptor_layout = lbuilder.build();
		}

		this->descriptor_pool = 
		{vk2::DescriptorPoolInfo{
			.limits =
			{
				.limits = 
				{
					{vk2::DescriptorType::StorageBuffer, buffer_count * RendererVulkan::max_frames_in_flight},
					{vk2::DescriptorType::ImageWithSampler, this->image_component_views.size() * RendererVulkan::max_frames_in_flight}
				},
				.max_sets = static_cast<std::uint32_t>(RendererVulkan::max_frames_in_flight),
				.supports_update_after_bind = true
			},
			.logical_device = &ldev
		}};
		{
			tz::BasicList<const vk2::DescriptorLayout*> alloc_layout_list;
			for(std::size_t i = 0; i < RendererVulkan::max_frames_in_flight; i++)
			{
				alloc_layout_list.add(&this->descriptor_layout);
			}
			this->descriptors = this->descriptor_pool.allocate_sets
			({
				.set_layouts = std::move(alloc_layout_list)
			});
		};
		tz_assert(this->descriptors.success(), "Descriptor Pool allocation failed. Please submit a bug report.");
		this->sync_descriptors(true);
	}

	const IComponent* ResourceStorage::get_component(ResourceHandle handle) const
	{
		return this->components[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))].get();
	}

	IComponent* ResourceStorage::get_component(ResourceHandle handle)
	{
		return this->components[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))].get();
	}

	const vk2::DescriptorLayout& ResourceStorage::get_descriptor_layout() const
	{
		return this->descriptor_layout;
	}

	std::span<const vk2::DescriptorSet> ResourceStorage::get_descriptor_sets() const
	{
		return this->descriptors.sets;
	}

	std::size_t ResourceStorage::resource_count_of(ResourceType type) const
	{
		return std::count_if(this->components.begin(), this->components.end(),
		[type](const auto& component_ptr)
		{
			return component_ptr->get_resource()->get_type() == type;
		});
	}

	void ResourceStorage::sync_descriptors(bool write_everything)
	{
		std::vector<vk2::Buffer*> buffers;
		for(auto& component_ptr : this->components)
		{
			if(component_ptr->get_resource()->get_type() == ResourceType::Buffer)
			{
				buffers.push_back(&static_cast<BufferComponentVulkan*>(component_ptr.get())->vk_get_buffer());
			}
		}

		// Now write the initial resources into their descriptors.
		vk2::DescriptorPool::UpdateRequest update = this->descriptor_pool.make_update_request();
		// For each set, make the same edits.
		for(std::size_t i = 0 ; i < RendererVulkan::max_frames_in_flight; i++)
		{
			vk2::DescriptorSet& set = this->descriptors.sets[i];
			vk2::DescriptorSet::EditRequest set_edit = set.make_edit_request();
			// Now update each binding corresponding to a buffer resource.
			for(std::size_t j = 0; j < buffers.size(); j++)
			{
				set_edit.set_buffer(j,
				{
					.buffer = buffers[j],
					.buffer_offset = 0,
					.buffer_write_size = buffers[j]->size()
				});
			}
			// And finally the binding corresponding to the texture resource descriptor array
			// Note, we only do this if we're writing to everything (we should only ever do this once because the image resources are not update-after-bind-bit!
			if(write_everything)
			{
				for(std::size_t j = 0; j < this->image_component_views.size(); j++)
				{
					set_edit.set_image(buffers.size(),
					{
						.sampler = &this->basic_sampler,
						.image_view = &this->image_component_views[j]
					}, j);
				}
			}
			update.add_set_edit(set_edit);
		}
		this->descriptor_pool.update_sets(update);
	}

//--------------------------------------------------------------------------------------------------

	OutputManager::OutputManager(IOutput* output, std::span<vk2::Image> window_buffer_images, bool create_depth_images, const vk2::LogicalDevice& ldev):
	output(output),
	window_buffer_images(window_buffer_images),
	window_buffer_depth_images(),
	output_imageviews(),
	render_pass(vk2::RenderPass::null()),
	output_framebuffers()
	{
		tz_assert(!this->get_output_images().empty(), "RendererVulkan OutputManager was not given any output images. Please submit a bug report.");
		this->window_buffer_depth_images.reserve(this->window_buffer_images.size());
		this->output_depth_imageviews.reserve(this->window_buffer_images.size());
		for(const vk2::Image& window_buffer_image : this->window_buffer_images)
		{
			// If we need depth images for depth testing, we'll create them based off of the existing output images we are provided from the Device.
			if(create_depth_images)
			{
				this->window_buffer_depth_images.push_back
				(vk2::ImageInfo{
					.device = &ldev,
					.format = vk2::ImageFormat::Depth16_UNorm,
					.dimensions = window_buffer_image.get_dimensions(),
					.usage = {vk2::ImageUsage::DepthStencilAttachment},
					.residency = vk2::MemoryResidency::GPU
				});
			}
			else
			{
				// Otherwise, just create empty ones and don't bother using them.
				this->window_buffer_depth_images.push_back(vk2::Image::null());
			}
		}
		for(std::size_t i = 0; i < this->get_output_images().size(); i++)
		{
			// We need to pass image views around in various places within the vulkan api. We create them here.
			this->output_imageviews.push_back
			(vk2::ImageViewInfo{
				.image = &this->get_output_images()[i],
				.aspect = vk2::ImageAspect::Colour
			});

			// If we made depth images earlier, create views for them too.
			if(create_depth_images)
			{
				this->output_depth_imageviews.push_back
				(vk2::ImageViewInfo{
					.image = &this->window_buffer_depth_images[i],
					.aspect = vk2::ImageAspect::Depth
				});
			}
			else
			{
				this->output_depth_imageviews.push_back(vk2::ImageView::null());
			}
		}

		tz_assert(std::equal(this->output_imageviews.begin(), this->output_imageviews.end(), this->output_imageviews.begin(), [](const vk2::ImageView& a, const vk2::ImageView& b){return a.get_image().get_format() == b.get_image().get_format();}), "Detected that not every output image in a RendererVulkan has the same format. This is not permitted as RenderPasses would not be compatible. Please submit a bug report.");

		// Now create an ultra-basic renderpass.
		// We're matching the ImageFormat of the provided output image.
		vk2::ImageLayout final_layout;
		if(this->output == nullptr || this->output->get_target() == OutputTarget::Window)
		{
			final_layout = vk2::ImageLayout::Present;
		}
		else if(this->output->get_target() == OutputTarget::OffscreenImage)
		{
			final_layout = vk2::ImageLayout::ColourAttachment;
		}
		else
		{
			final_layout = vk2::ImageLayout::Undefined;
			tz_error("Unknown RendererOutputType. Please submit a bug report.");
		}

		vk2::RenderPassBuilder rbuilder;
		rbuilder.set_device(ldev);
		rbuilder.with_attachment
		({
			.format = this->get_output_images().front().get_format(),
			.colour_depth_store = vk2::StoreOp::Store,
			.initial_layout = vk2::ImageLayout::Undefined,
			.final_layout = final_layout
		});
		if(create_depth_images)
		{
			rbuilder.with_attachment
			({
				.format = this->window_buffer_depth_images.front().get_format(),
				.colour_depth_store = vk2::StoreOp::DontCare,
				.initial_layout = vk2::ImageLayout::Undefined,
				.final_layout = vk2::ImageLayout::DepthStencilAttachment
			});
		}

		vk2::SubpassBuilder sbuilder;
		sbuilder.set_pipeline_context(vk2::PipelineContext::Graphics);
		sbuilder.with_colour_attachment
		({
			.attachment_idx = 0,
			.current_layout = vk2::ImageLayout::ColourAttachment
		});
		if(create_depth_images)
		{
			sbuilder.with_depth_stencil_attachment
			({
				.attachment_idx = 1,
				.current_layout = vk2::ImageLayout::DepthStencilAttachment
			});
		}

		this->render_pass = rbuilder.with_subpass(sbuilder.build()).build();

		for(std::size_t i = 0; i < this->output_imageviews.size(); i++)
		{
			tz::Vec2ui dims = this->output_imageviews[i].get_image().get_dimensions();
			tz::BasicList<vk2::ImageView*> attachments{&this->output_imageviews[i]};
			if(create_depth_images)
			{
				attachments.add(&this->output_depth_imageviews[i]);
			}
			this->output_framebuffers.push_back
			(vk2::FramebufferInfo{
				.render_pass = &this->render_pass,
				.attachments = attachments,
				.dimensions = dims
			});
		}
	}

	const vk2::RenderPass& OutputManager::get_render_pass() const
	{
		return this->render_pass;
	}

	std::span<vk2::Image> OutputManager::get_output_images()
	{
		if(this->output == nullptr || this->output->get_target() == OutputTarget::Window)
		{
			// We're rendering into a window (which may be headless). The Device contained the swapchain images (or the offline headless images). Simply return those.
			return this->window_buffer_images;
		}
		else if(this->output->get_target() == OutputTarget::OffscreenImage)
		{
			// We have been provided an ImageOutput which will contain an ImageComponentVulkan. We need to retrieve that image and return a span covering it.
			// TODO: Support multiple-render-targets.
			ImageOutput& out = static_cast<ImageOutput&>(*this->output);
			vk2::Image& output_image = out.get_component().vk_get_image();
			return {&output_image, 1};
		}
		else
		{
			tz_error("Unrecognised OutputTarget. Please submit a bug report.");
			return {};
		}
	}

	std::span<const vk2::Framebuffer> OutputManager::get_output_framebuffers() const
	{
		return this->output_framebuffers;
	}

	std::span<vk2::Framebuffer> OutputManager::get_output_framebuffers()
	{
		return this->output_framebuffers;
	}

	tz::Vec2ui OutputManager::get_output_dimensions() const
	{
		tz_assert(!this->output_imageviews.empty(), "OutputManager had no output views, so impossible to retrieve viewport dimensions. Please submit a bug report.");
		return this->output_imageviews.front().get_image().get_dimensions();
	}

//--------------------------------------------------------------------------------------------------

	GraphicsPipelineManager::GraphicsPipelineManager
	(
		const ShaderInfo& sinfo,
		const vk2::DescriptorLayout& dlayout,
		const vk2::RenderPass& render_pass,
		std::size_t frame_in_flight_count,
		tz::Vec2ui viewport_dimensions,
		bool depth_testing_enabled
	):
	shader(this->make_shader(dlayout.get_device(), sinfo)),
	pipeline_layout(this->make_pipeline_layout(dlayout, frame_in_flight_count)),
	graphics_pipeline
	({
		.shaders = this->shader.native_data(),
		.state = vk2::PipelineState
		{
			.viewport = vk2::create_basic_viewport({static_cast<float>(viewport_dimensions[0]), static_cast<float>(viewport_dimensions[1])}),
			.depth_stencil =
			{
				.depth_testing = depth_testing_enabled,
				.depth_writes = depth_testing_enabled,
			}
		},
		.pipeline_layout = &this->pipeline_layout,
		.render_pass = &render_pass,
		.device = &render_pass.get_device()
	})
	{
		// TODO: Implement vk2::LogicalDevice equality operator
		//tz_assert(dlayout.get_device() == render_pass.get_device(), "");
	}

	const vk2::GraphicsPipeline& GraphicsPipelineManager::get_pipeline() const
	{
		return this->graphics_pipeline;
	}

	vk2::Shader GraphicsPipelineManager::make_shader(const vk2::LogicalDevice& ldev, const ShaderInfo& sinfo) const
	{
		std::vector<char> vtx_src, frg_src, cmp_src;
		tz::BasicList<vk2::ShaderModuleInfo> modules;
		if(sinfo.has_shader(ShaderStage::Compute))
		{
			// Compute, we only care about the compute shader.
			{
				std::string_view compute_source = sinfo.get_shader(ShaderStage::Compute);
				cmp_src.resize(compute_source.length());
				std::copy(compute_source.begin(), compute_source.end(), cmp_src.begin());
			}
			modules =
			{
				{
					.device = &ldev,
					.type = vk2::ShaderType::Compute,
					.code = cmp_src
				}
			};
		}
		else
		{
			// Graphics, must contain a Vertex and Fragment shader.
			tz_assert(sinfo.has_shader(ShaderStage::Vertex), "ShaderInfo must contain a non-empty vertex shader if no compute shader is present.");
			tz_assert(sinfo.has_shader(ShaderStage::Fragment), "ShaderInfo must contain a non-empty fragment shader if no compute shader is present.");
			{
				std::string_view vertex_source = sinfo.get_shader(ShaderStage::Vertex);
				vtx_src.resize(vertex_source.length());
				std::copy(vertex_source.begin(), vertex_source.end(), vtx_src.begin());

				std::string_view fragment_source = sinfo.get_shader(ShaderStage::Fragment);
				frg_src.resize(fragment_source.length());
				std::copy(fragment_source.begin(), fragment_source.end(), frg_src.begin());
			}
			modules = 
			{
				{
					.device = &ldev,
					.type = vk2::ShaderType::Vertex,
					.code = vtx_src
				},
				{
					.device = &ldev,
					.type = vk2::ShaderType::Fragment,
					.code = frg_src
				}
			};
		}
		return
		{{
			.device = &ldev,
			.modules = modules
		}};
	}

	vk2::PipelineLayout GraphicsPipelineManager::make_pipeline_layout(const vk2::DescriptorLayout& dlayout, std::size_t frame_in_flight_count) const
	{
		std::vector<const vk2::DescriptorLayout*> layout_ptrs(frame_in_flight_count, &dlayout);
		return
		{{
			.descriptor_layouts = std::move(layout_ptrs),
			.logical_device = &dlayout.get_device()
		}};
	}

//--------------------------------------------------------------------------------------------------

	CommandProcessor::CommandProcessor(vk2::LogicalDevice& ldev, std::size_t frame_in_flight_count, OutputTarget output_target, std::span<vk2::Framebuffer> output_framebuffers):
	requires_present(output_target == OutputTarget::Window),
	graphics_queue(ldev.get_hardware_queue
	({
		.field = {vk2::QueueFamilyType::Graphics},
		.present_support = this->requires_present
	})),
	command_pool
	({
		.queue = this->graphics_queue,
		.flags = {vk2::CommandPoolFlag::Reusable}
	}),
	commands(this->command_pool.allocate_buffers
	({
		.buffer_count = static_cast<std::uint32_t>(frame_in_flight_count + 1)
	})),
	frame_in_flight_count(frame_in_flight_count),
	image_semaphores(),
	render_work_semaphores(),
	in_flight_fences(),
	images_in_flight(this->frame_in_flight_count, nullptr)
	{
		tz_assert(output_framebuffers.size() == this->frame_in_flight_count, "Provided incorrect number of output framebuffers. We must have enough framebuffers for each frame we have in flight. Provided %zu framebuffers, but need %zu because that's how many frames we have in flight.", output_framebuffers.size(), this->frame_in_flight_count);
		tz_assert(this->commands.success(), "Failed to allocate from CommandPool");
		for(std::size_t i = 0; i < this->frame_in_flight_count; i++)
		{
			this->image_semaphores.emplace_back(ldev);
			this->render_work_semaphores.emplace_back(ldev);
			this->in_flight_fences.emplace_back
			(vk2::FenceInfo{
				.device = &ldev,
				.initially_signalled = true
			});
		}
	}

	std::span<const vk2::CommandBuffer> CommandProcessor::get_render_command_buffers() const
	{
		return {this->commands.buffers.begin(), this->frame_in_flight_count};
	}

	std::span<vk2::CommandBuffer> CommandProcessor::get_render_command_buffers()
	{
		return {this->commands.buffers.begin(), this->frame_in_flight_count};
	}

	CommandProcessor::RenderWorkSubmitResult CommandProcessor::do_render_work(vk2::Swapchain* maybe_swapchain)
	{
		if(this->requires_present)
		{
			tz_assert(maybe_swapchain != nullptr, "Trying to do render work with presentation, but no Swapchain provided. Please submit a bug report.");
			vk2::Swapchain& swapchain = *maybe_swapchain;
			// Submit & Present
			this->in_flight_fences[this->current_frame].wait_until_signalled();
			this->output_image_index = swapchain.acquire_image
			({
				.signal_semaphore = &this->image_semaphores[current_frame]
			}).image_index;

			const vk2::Fence*& target_image = this->images_in_flight[this->output_image_index];
			if(target_image != nullptr)
			{
				target_image->wait_until_signalled();
			}
			target_image = &this->in_flight_fences[this->output_image_index];

			this->in_flight_fences[this->current_frame].unsignal();
			this->graphics_queue->submit
			({
				.command_buffers = {&this->get_render_command_buffers()[this->output_image_index]},
				.waits =
				{
					vk2::hardware::Queue::SubmitInfo::WaitInfo
					{
						.wait_semaphore = &this->image_semaphores[this->current_frame],
						.wait_stage = vk2::PipelineStage::ColourAttachmentOutput
					}
				},
				.signal_semaphores = {&this->render_work_semaphores[this->current_frame]},
				.execution_complete_fence = &this->in_flight_fences[this->current_frame]
			});

			CommandProcessor::RenderWorkSubmitResult result;

			result.present = this->graphics_queue->present
			({
				.wait_semaphores = {&this->render_work_semaphores[this->current_frame]},
				.swapchain = maybe_swapchain,
				.swapchain_image_index = this->output_image_index
			});
			this->current_frame = (this->current_frame + 1) % this->frame_in_flight_count;
			return result;
		}
		else
		{
			// Headlessly
			tz_error("Headless rendering not yet implemented.");
			return {.present = vk2::hardware::Queue::PresentResult::Fail_FatalError};
		}
	}

	void CommandProcessor::wait_pending_commands_complete()
	{
		for(const vk2::Fence& fence : this->in_flight_fences)
		{
			fence.wait_until_signalled();
		}
	}

//--------------------------------------------------------------------------------------------------

	RendererVulkan::RendererVulkan(RendererInfoVulkan& info, const RendererDeviceInfoVulkan& device_info):
	ldev(device_info.device),
	resources(info.get_resources(), *this->ldev),
	output(info.get_output(), device_info.output_images, !info.get_options().contains(RendererOption::NoDepthTesting), *this->ldev),
	pipeline(info.shader(), this->resources.get_descriptor_layout(), this->output.get_render_pass(), RendererVulkan::max_frames_in_flight, output.get_output_dimensions(), !info.get_options().contains(RendererOption::NoDepthTesting)),
	command(*this->ldev, RendererVulkan::max_frames_in_flight, info.get_output() != nullptr ? info.get_output()->get_target() : OutputTarget::Window, this->output.get_output_framebuffers()),
	maybe_swapchain(device_info.maybe_swapchain),
	options(info.get_options())
	{
		this->setup_static_resources();
		this->setup_render_commands();
	}

	RendererVulkan::~RendererVulkan()
	{
		this->ldev->wait_until_idle();
	}

	unsigned int RendererVulkan::resource_count() const
	{
		return this->resources.count();
	}

	const IResource* RendererVulkan::get_resource(ResourceHandle handle) const
	{
		return this->resources.get(handle);
	}

	IResource* RendererVulkan::get_resource(ResourceHandle handle)
	{
		return this->resources.get(handle);
	}

	const IComponent* RendererVulkan::get_component(ResourceHandle handle) const
	{
		return this->resources.get_component(handle);
	}

	IComponent* RendererVulkan::get_component(ResourceHandle handle)
	{
		return this->resources.get_component(handle);
	}

	const RendererOptions& RendererVulkan::get_options() const
	{
		return this->options;
	}

	void RendererVulkan::render()
	{
		CommandProcessor::RenderWorkSubmitResult result = this->command.do_render_work(this->maybe_swapchain);
		switch(result.present)
		{
			case vk2::hardware::Queue::PresentResult::Success_Suboptimal:
			[[fallthrough]];
			case vk2::hardware::Queue::PresentResult::Success:

			break;
			default:
				// TODO: this->handle_resize();
				tz_error("Presentation Failed. You probably tried to resize/minimise the window, but support for this on Vulkan is not yet implemented.");
			break;
		}
	}

	void RendererVulkan::render(unsigned int tri_count)
	{
		if(this->tri_count != tri_count)
		{
			this->tri_count = tri_count;
			this->command.wait_pending_commands_complete();
			this->setup_render_commands();
		}
		this->render();
	}

	void RendererVulkan::edit(const RendererEditRequest& edit_request)
	{
		if(edit_request.component_edits.empty())
		{
			return;
		}
		// We have buffer/image components we need to edit.
		// Firstly make sure all render work is done, then we need to update the resource storage and then write new descriptors for resized resources.
		this->command.wait_pending_commands_complete();
		// Now, if we resized any static resources we're going to have to run scratch commands again.
		bool resized_static_resources = false;
		for(const RendererComponentEditRequest& component_edit : edit_request.component_edits)
		{
			std::visit(
			[this](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, RendererBufferComponentEditRequest>)
				{
					// We need to create a new buffer with the correct size, and swap it with the old one (which can now die because we're not rendering anymore).
					// If it's a dynamic resource, we need to unmap the BufferResource from it though.
					auto buf_comp = static_cast<BufferComponentVulkan*>(this->get_component(arg.buffer_handle));
					tz_assert(buf_comp->get_resource()->get_access() == ResourceAccess::DynamicVariable, "Detected attempted resize of buffer resource (id %zu), but it ResourceAccess is not DynamicVariable. This means it is a fixed-size resource, so attempting to resize it is invalid.", static_cast<std::size_t>(static_cast<tz::HandleValue>(arg.buffer_handle)));
					// Make new buffer.
					vk2::Buffer& buf = buf_comp->vk_get_buffer();
					vk2::Buffer resized_copy
					{{
						.device = &buf.get_device(),
						.size_bytes = arg.size,
						.usage = buf.get_usage(),
						.residency = buf.get_residency()
					}};
					// Swap them. If we're dynamic, we move the data over now, otherwise we will sort that later when we setup static resources again.
					if(buf_comp->get_resource()->get_access() == ResourceAccess::DynamicFixed || buf_comp->get_resource()->get_access() == ResourceAccess::DynamicVariable)
					{
						// Unmap resource from previous buffer component, and instead map it to this one. But first we want to copy over the old data.
						std::span<const std::byte> old_data = buf.map_as<const std::byte>();
						std::span<std::byte> new_data = resized_copy.map_as<std::byte>();
						// Assume new resource is larger than old one.
						tz_assert(old_data.size_bytes() <= new_data.size_bytes(), "Buffer component resizing to a smaller size is not yet implemented.");
						// do the copy.
						std::copy(old_data.begin(), old_data.end(), new_data.begin());
						// Now remap the resource to the new component. Then we can swap.
						buf_comp->get_resource()->set_mapped_data(new_data);
					}
					// Swap the buffers, old one can now die. Remember if we're static the data can't change, so the initial resource data is still correct so we don't have to do anything.
					std::swap(buf, resized_copy);
					// Now update all descriptors.
					this->resources.sync_descriptors(false);
				}
				else if constexpr(std::is_same_v<T, RendererImageComponentEditRequest>)
				{
					tz_error("Resizing images is not yet implemented");
				}
				else
				{
					tz_error("Unsupported Variant Type");
				}
			}, component_edit);
		}
		if(resized_static_resources)
		{
			this->setup_static_resources();
		}
	}

	void RendererVulkan::setup_static_resources()
	{
		// Create staging buffers for each buffer and texture resource, and then fill the data with the resource data.
		std::vector<BufferComponentVulkan*> buffer_components;
		std::vector<ImageComponentVulkan*> image_components;
		for(std::size_t i = 0; i < this->resource_count(); i++)
		{
			IComponent* icomp = this->get_component(static_cast<tz::HandleValue>(i));
			switch(icomp->get_resource()->get_type())
			{
				case ResourceType::Buffer:
					buffer_components.push_back(static_cast<BufferComponentVulkan*>(icomp));
				break;
				case ResourceType::Image:
					image_components.push_back(static_cast<ImageComponentVulkan*>(icomp));
				break;
				default:
					tz_error("Unknown ResourceType.");
				break;
			}
		}

		// Staging buffers.
		std::vector<vk2::Buffer> staging_buffers;
		staging_buffers.reserve(buffer_components.size());
		std::vector<vk2::Buffer> staging_image_buffers;
		staging_image_buffers.reserve(image_components.size());
		// Fill buffers with resource data.
		std::transform(buffer_components.begin(), buffer_components.end(), std::back_inserter(staging_buffers),
		[](const BufferComponentVulkan* buf)->vk2::Buffer
		{
			return
			{{
				.device = &buf->vk_get_buffer().get_device(),
				.size_bytes = buf->size(),
				.usage = {vk2::BufferUsage::TransferSource},
				.residency = vk2::MemoryResidency::CPU
			}};
		});
		// Same with images.
		std::transform(image_components.begin(), image_components.end(), std::back_inserter(staging_image_buffers),
		[](const ImageComponentVulkan* img)->vk2::Buffer
		{
			return
			{{
				.device = &img->vk_get_image().get_device(),
				.size_bytes = tz::gl2::pixel_size_bytes(img->get_format()) * img->get_dimensions()[0] * img->get_dimensions()[1],
				.usage = {vk2::BufferUsage::TransferSource},
				.residency = vk2::MemoryResidency::CPU
			}};
		});
		this->command.do_scratch_operations([&staging_buffers, &staging_image_buffers, &buffer_components, &image_components](vk2::CommandBufferRecording& recording)
		{
			// Finally, upload data for static resources.
			for(std::size_t i = 0; i < buffer_components.size(); i++)
			{
				IResource* res = buffer_components[i]->get_resource();
				tz_assert(res->get_type() == ResourceType::Buffer, "Expected ResourceType of buffer, but is not a buffer. Please submit a bug report.");
				if(res->get_access() != ResourceAccess::StaticFixed)
				{
					continue;
				}

				// Now simply write the data straight in.
				{
					void* ptr = staging_buffers[i].map();
					std::memcpy(ptr, res->data().data(), res->data().size_bytes());
					staging_buffers[i].unmap();
				}

				// Record the command to transfer to the buffer resource.
				recording.buffer_copy_buffer
				({
					.src = &staging_buffers[i],
					.dst = &buffer_components[i]->vk_get_buffer()
				});
			}
			for(std::size_t i = 0; i < image_components.size(); i++)
			{
				IResource* res = image_components[i]->get_resource();
				tz_assert(res->get_type() == ResourceType::Image, "Expected ResourceType of Texture, but is not a texture. Please submit a bug report.");
				if(res->get_access() != ResourceAccess::StaticFixed)
				{
					continue;
				}
				// Now simply write the data straight in.
				{
					void* ptr = staging_image_buffers[i].map();
					std::memcpy(ptr, res->data().data(), res->data().size_bytes());
					staging_image_buffers[i].unmap();
				}
				// Record the command to transfer to the texture resource.
				// Image will initially be in undefined layout. We need to:
				// - Transition the texture component to TransferDestination
				// - Transfer from the staging texture buffer
				// - Transition the texture component to ShaderResource so it can be used in the shader.
				recording.transition_image_layout
				({
					.image = &image_components[i]->vk_get_image(),
					.target_layout = vk2::ImageLayout::TransferDestination,
					.source_access = {vk2::AccessFlag::None},
					.destination_access = {vk2::AccessFlag::TransferOperationWrite},
					.source_stage = vk2::PipelineStage::Top,
					.destination_stage = vk2::PipelineStage::TransferCommands,
					.image_aspects = {vk2::ImageAspectFlag::Colour}
				});
				recording.buffer_copy_image
				({
					.src = &staging_image_buffers[i],
					.dst = &image_components[i]->vk_get_image(),
					.image_aspects = {vk2::ImageAspectFlag::Colour}
				});
				recording.transition_image_layout
				({
					.image = &image_components[i]->vk_get_image(),
					.target_layout = vk2::ImageLayout::ShaderResource,
					.source_access = {vk2::AccessFlag::TransferOperationWrite},
					.destination_access = {vk2::AccessFlag::ShaderResourceRead},
					.source_stage = vk2::PipelineStage::TransferCommands,
					.destination_stage = vk2::PipelineStage::FragmentShader,
					.image_aspects = {vk2::ImageAspectFlag::Colour}
				});
			}
		});
	}

	void RendererVulkan::setup_render_commands()
	{
		this->command.set_rendering_commands([this](vk2::CommandBufferRecording& recording, std::size_t framebuffer_id)
		{
			tz_assert(framebuffer_id < this->output.get_output_framebuffers().size(), "Attempted to retrieve output framebuffer at index %zu, but there are only %zu framebuffers available. Please submit a bug report.", framebuffer_id, this->output.get_output_framebuffers().size());
			vk2::CommandBufferRecording::RenderPassRun run{this->output.get_output_framebuffers()[framebuffer_id], recording};
			recording.bind_pipeline
			({
				.pipeline = &this->pipeline.get_pipeline(),
				.pipeline_context = vk2::PipelineContext::Graphics
			});
			tz::BasicList<const vk2::DescriptorSet*> sets;
			std::span<const vk2::DescriptorSet> resource_sets = this->resources.get_descriptor_sets();
			sets.resize(resource_sets.size());
			std::transform(resource_sets.begin(), resource_sets.end(), sets.begin(), [](const vk2::DescriptorSet& set){return &set;});
			recording.bind_descriptor_sets
			({
				.pipeline_layout = this->pipeline.get_pipeline().get_info().pipeline_layout,
				.context = vk2::PipelineContext::Graphics,
				.descriptor_sets = sets,
				.first_set_id = 0
			});
			recording.draw
			({
				.vertex_count = 3 * this->tri_count,
				.instance_count = 1,
				.first_vertex = 0,
				.first_instance = 0
			});
		});
	}
}

#endif // TZ_VULKAN
