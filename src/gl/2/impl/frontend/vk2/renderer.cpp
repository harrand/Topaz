#include "gl/impl/backend/vk2/fixed_function.hpp"
#if TZ_VULKAN
#include "gl/impl/backend/vk2/descriptors.hpp"
#include "gl/impl/backend/vk2/image_view.hpp"
#include "gl/2/impl/frontend/vk2/renderer.hpp"
#include "gl/2/impl/frontend/vk2/component.hpp"
#include "gl/2/output.hpp"

namespace tz::gl2
{
	using namespace tz::gl;

	ResourceStorage::ResourceStorage(std::span<const IResource* const> resources, const vk2::LogicalDevice& ldev):
	AssetStorage<IResource>(resources),
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
		for(std::size_t i = 0; i < this->count(); i++)
		{
			IResource* res = this->get(static_cast<tz::HandleValue>(i));
			switch(res->get_type())
			{
				case ResourceType::Buffer:
					this->components.push_back(std::make_unique<BufferComponentVulkan>(*res, ldev));
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
		std::vector<vk2::Buffer*> buffers;
		for(const auto& component_ptr : this->components)
		{
			if(component_ptr->get_resource()->get_type() != ResourceType::Buffer)
			{
				continue;
			}
			buffers.push_back(&static_cast<BufferComponentVulkan*>(component_ptr.get())->vk_get_buffer());
		}
		{
			vk2::DescriptorLayoutBuilder lbuilder;
			lbuilder.set_device(ldev);
			// Each buffer gets their own binding id.
			for(std::size_t i = 0; i < buffers.size(); i++)
			{
				lbuilder.with_binding
				({
					.type = vk2::DescriptorType::StorageBuffer,
					.count = 1
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
					{vk2::DescriptorType::StorageBuffer, buffers.size() * RendererVulkan::max_frames_in_flight},
					{vk2::DescriptorType::ImageWithSampler, this->image_component_views.size() * RendererVulkan::max_frames_in_flight}
				},
				.max_sets = static_cast<std::uint32_t>(RendererVulkan::max_frames_in_flight)
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
			for(std::size_t j = 0; j < this->image_component_views.size(); j++)
			{
				set_edit.set_image(buffers.size(),
				{
					.sampler = &this->basic_sampler,
					.image_view = &this->image_component_views[j]
				}, j);
			}
			update.add_set_edit(set_edit);
		}
		this->descriptor_pool.update_sets(update);
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

//--------------------------------------------------------------------------------------------------
	unsigned int RendererInfoVulkan::input_count() const
	{
		return this->inputs.size();
	}

	const IInput* RendererInfoVulkan::get_input(InputHandle handle)
	{
		return this->inputs[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))];
	}

	std::span<const IInput* const> RendererInfoVulkan::get_inputs() const
	{
		return this->inputs;
	}

	unsigned int RendererInfoVulkan::resource_count() const
	{
		return this->resources.size();
	}

	const IResource* RendererInfoVulkan::get_resource(ResourceHandle handle)
	{
		return this->resources[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))];
	}

	std::span<const IResource* const> RendererInfoVulkan::get_resources() const
	{
		return this->resources;
	}

	InputHandle RendererInfoVulkan::add_input(IInput& input)
	{
		this->inputs.push_back(&input);
		return static_cast<tz::HandleValue>(this->inputs.size() - 1);
	}

	ResourceHandle RendererInfoVulkan::add_resource(IResource& resource)
	{
		this->resources.push_back(&resource);
		return static_cast<tz::HandleValue>(this->resources.size() - 1);
	}

	void RendererInfoVulkan::set_output(IOutput& output)
	{
		this->output = &output;
	}

	const IOutput* RendererInfoVulkan::get_output() const
	{
		return this->output;
	}

	IOutput* RendererInfoVulkan::get_output()
	{
		return this->output;
	}

	ShaderInfo& RendererInfoVulkan::shader()
	{
		return this->shader_info;
	}

	const ShaderInfo& RendererInfoVulkan::shader() const
	{
		return this->shader_info;
	}

//--------------------------------------------------------------------------------------------------

	OutputManager::OutputManager(IOutput* output, std::span<vk2::Image> window_buffer_images, const vk2::LogicalDevice& ldev):
	output(output),
	window_buffer_images(window_buffer_images),
	output_imageviews(),
	render_pass(vk2::RenderPass::null()),
	output_framebuffers()
	{
		tz_assert(!this->get_output_images().empty(), "RendererVulkan OutputManager was not given any output images. Please submit a bug report.");
		for(vk2::Image& output_image : this->get_output_images())
		{
			this->output_imageviews.push_back
			(vk2::ImageViewInfo{
				.image = &output_image,
				.aspect = vk2::ImageAspect::Colour
			});
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
			.initial_layout = vk2::ImageLayout::Undefined,
			.final_layout = final_layout
		});

		vk2::SubpassBuilder sbuilder;
		sbuilder.set_pipeline_context(vk2::PipelineContext::Graphics);
		sbuilder.with_colour_attachment
		({
			.attachment_idx = 0,
			.current_layout = vk2::ImageLayout::ColourAttachment
		});

		this->render_pass = rbuilder.with_subpass(sbuilder.build()).build();
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
		tz::Vec2ui viewport_dimensions
	):
	shader(this->make_shader(dlayout.get_device(), sinfo)),
	pipeline_layout(this->make_pipeline_layout(dlayout, frame_in_flight_count)),
	graphics_pipeline
	({
		.shaders = this->shader.native_data(),
		.state = vk2::PipelineState{.viewport = vk2::create_basic_viewport({static_cast<float>(viewport_dimensions[0]), static_cast<float>(viewport_dimensions[1])})},
		.pipeline_layout = &this->pipeline_layout,
		.render_pass = &render_pass,
		.device = &render_pass.get_device()
	})
	{
		// TODO: Implement vk2::LogicalDevice equality operator
		//tz_assert(dlayout.get_device() == render_pass.get_device(), "");
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

	RendererVulkan::RendererVulkan(RendererInfoVulkan& info, const RendererDeviceInfoVulkan& device_info):
	inputs(info.get_inputs()),
	resources(info.get_resources(), *device_info.device),
	output(info.get_output(), device_info.output_images, *device_info.device),
	pipeline(info.shader(), this->resources.get_descriptor_layout(), this->output.get_render_pass(), RendererVulkan::max_frames_in_flight, output.get_output_dimensions())
	{
		int x = 5;
	}

	unsigned int RendererVulkan::input_count() const
	{
		return this->inputs.count();
	}

	const IInput* RendererVulkan::get_input(InputHandle handle) const
	{
		return this->inputs.get(handle);
	}

	IInput* RendererVulkan::get_input(InputHandle handle)
	{
		return this->inputs.get(handle);
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
}

#endif // TZ_VULKAN
