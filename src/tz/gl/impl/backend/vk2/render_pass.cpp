#if TZ_VULKAN
#include "tz/core/profiling/zone.hpp"
#include "tz/gl/impl/backend/vk2/render_pass.hpp"
#include <numeric>

namespace tz::gl::vk2
{
	std::size_t RenderPassInfo::total_input_attachment_count() const
	{
		return std::accumulate(this->subpasses.begin(), this->subpasses.end(), 0,
		[](std::size_t cum, const Subpass& subpass)
		{
			return cum + subpass.input_attachments.length();
		});
	}

	std::size_t RenderPassInfo::total_colour_attachment_count() const
	{
		return std::accumulate(this->subpasses.begin(), this->subpasses.end(), 0,
		[](std::size_t cum, const Subpass& subpass)
		{
			return cum + subpass.colour_attachments.length();
		});
	}

	bool RenderPassInfo::has_valid_device() const
	{
		return this->logical_device != nullptr && !this->logical_device->is_null();
	}

	bool RenderPassInfo::values_make_sense() const
	{
		std::size_t num_attachments = this->attachments.length();
		auto stays_in_range = [num_attachments](const AttachmentReference& ref) -> bool
		{
			return ref.attachment_idx < num_attachments;
		};

		bool sane = true;
		sane &= std::all_of(this->subpasses.begin(), this->subpasses.end(),
		[stays_in_range](const RenderPassInfo::Subpass& subpass)->bool
		{
			bool depth_in_range_if_exist = true;
			if(subpass.depth_stencil_attachment.has_value())
			{
				depth_in_range_if_exist = stays_in_range(subpass.depth_stencil_attachment.value());
			}

			auto input_stays_in_range = [stays_in_range](const RenderPassInfo::InputAttachmentReference& input_ref)->bool{return stays_in_range(input_ref.attachment_ref);};
			return std::all_of(subpass.input_attachments.begin(), subpass.input_attachments.end(), input_stays_in_range)
			&& std::all_of(subpass.colour_attachments.begin(), subpass.colour_attachments.end(), stays_in_range) && depth_in_range_if_exist;
		});

		return sane;
	}

	RenderPass::RenderPass(RenderPassInfo info):
	pass(VK_NULL_HANDLE),
	info(info)
	{
		TZ_PROFZONE("Vulkan Backend - RenderPass Create", TZ_PROFCOL_RED);
		hdk::assert(this->info.has_valid_device(), "RenderPassInfo contained nullptr or null LogicalDevice. Please submit a bug report.");
		hdk::assert(this->info.values_make_sense(), "RenderPassInfo contained values which didn't pass sanity checks. This will most likely crash horribly. Please submit a bug report. Most likely candidate is one of the attachment references contains an attachment id out of range.");
		std::vector<VkAttachmentDescription2> attachment_natives(info.attachments.length());
		std::transform(info.attachments.begin(), info.attachments.end(), attachment_natives.begin(),
		[](const Attachment& attachment) -> VkAttachmentDescription2
		{
			return
			{
				.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.pNext = nullptr,
				.flags = 0,
				.format = static_cast<VkFormat>(attachment.format),
				.samples = static_cast<VkSampleCountFlagBits>(attachment.sample_count),
				.loadOp = static_cast<VkAttachmentLoadOp>(attachment.colour_depth_load),
				.storeOp = static_cast<VkAttachmentStoreOp>(attachment.colour_depth_store),
				.stencilLoadOp = static_cast<VkAttachmentLoadOp>(attachment.stencil_load),
				.stencilStoreOp = static_cast<VkAttachmentStoreOp>(attachment.stencil_store),
				.initialLayout = static_cast<VkImageLayout>(attachment.initial_layout),
				.finalLayout = static_cast<VkImageLayout>(attachment.final_layout)
			};
		});

		// The VKAPI stuff gets really painful here, lots of pointers within pointers. We want one big vector for all, so we find the upper bound and reserve here.
		std::vector<VkAttachmentReference2> input_attachment_natives(info.total_input_attachment_count());
		std::vector<VkAttachmentReference2> colour_attachment_natives(info.total_colour_attachment_count());
		std::vector<std::optional<VkAttachmentReference2>> depth_attachment_natives(info.subpasses.length(), std::nullopt);

		std::vector<VkSubpassDescription2> subpass_natives(info.subpasses.length());
		struct AttachmentOffset
		{
			std::size_t input = 0;
			std::size_t colour = 0;
			std::size_t depth_stencil = 0;
		};
		AttachmentOffset offset;
		std::transform(info.subpasses.begin(), info.subpasses.end(), subpass_natives.begin(),
		[&offset, &input_attachment_natives, &colour_attachment_natives, &depth_attachment_natives](const RenderPassInfo::Subpass& subpass) -> VkSubpassDescription2
		{
			// Fill in the attachment natives.
			auto input_begin = input_attachment_natives.begin() + offset.input;
			std::transform(subpass.input_attachments.begin(), subpass.input_attachments.end(), input_begin,
			[](const RenderPassInfo::InputAttachmentReference& ref)->VkAttachmentReference2
			{
				return
				{
					.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
					.pNext = nullptr,
					.attachment = ref.attachment_ref.attachment_idx,
					.layout = static_cast<VkImageLayout>(ref.attachment_ref.current_layout),
					.aspectMask = static_cast<VkImageAspectFlags>(static_cast<ImageAspectFlag>(ref.aspect_flags))
				};
			});

			// Now the colour natives.
			auto colour_begin = colour_attachment_natives.begin() + offset.colour;
			std::transform(subpass.colour_attachments.begin(), subpass.colour_attachments.end(), colour_begin,
			[](const RenderPassInfo::AttachmentReference& ref)->VkAttachmentReference2
			{
				return
				{
					.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
					.pNext = nullptr,
					.attachment = ref.attachment_idx,
					.layout = static_cast<VkImageLayout>(ref.current_layout),
					.aspectMask = 0
				};
			});

			const std::optional<RenderPassInfo::AttachmentReference>& maybe_depth_attachment_ref = subpass.depth_stencil_attachment;

			// Then, the depth natives.
			if(maybe_depth_attachment_ref.has_value())
			{
				std::uint32_t att_idx = VK_ATTACHMENT_UNUSED;
				VkImageLayout depth_stencil_layout = static_cast<VkImageLayout>(ImageLayout::Undefined);
				if(maybe_depth_attachment_ref.has_value())
				{
					att_idx = maybe_depth_attachment_ref.value().attachment_idx;
					depth_stencil_layout = static_cast<VkImageLayout>(maybe_depth_attachment_ref.value().current_layout);
				}
				depth_attachment_natives[offset.depth_stencil] = 
				{
					.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
					.pNext = nullptr,
					.attachment = att_idx,
					.layout = depth_stencil_layout,
					.aspectMask = 0
				};
			}

			// Finally, write to the create natives.
			const std::optional<VkAttachmentReference2>& maybe_depth = depth_attachment_natives[offset.depth_stencil];
			VkSubpassDescription2 desc
			{
				.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
				.pNext = nullptr,
				.flags = 0,
				.pipelineBindPoint = static_cast<VkPipelineBindPoint>(subpass.context),
				.viewMask = 0,
				.inputAttachmentCount = static_cast<std::uint32_t>(subpass.input_attachments.length()),
				.pInputAttachments = input_attachment_natives.data() + offset.input,
				.colorAttachmentCount = static_cast<std::uint32_t>(subpass.colour_attachments.length()),
				.pColorAttachments = colour_attachment_natives.data() + offset.colour,
				.pResolveAttachments = nullptr,
				.pDepthStencilAttachment = maybe_depth.has_value() ? &maybe_depth.value() : nullptr,
				.preserveAttachmentCount = 0,
				.pPreserveAttachments = nullptr
			};
			offset.input += subpass.input_attachments.length();
			offset.colour += subpass.colour_attachments.length();
			offset.depth_stencil += subpass.depth_stencil_attachment.has_value() ? 1 : 0;
			return desc;
		});

		VkSubpassDependency2 dep
		{
			.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
			.pNext = nullptr,
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = 0,
			.viewOffset = 0
		};

		VkRenderPassCreateInfo2 create
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
			.pNext = nullptr,
			.flags = 0,
			.attachmentCount = static_cast<std::uint32_t>(attachment_natives.size()),
			.pAttachments = attachment_natives.data(),
			.subpassCount = static_cast<std::uint32_t>(subpass_natives.size()),
			.pSubpasses = subpass_natives.data(),
			.dependencyCount = 1,
			.pDependencies = &dep,
			.correlatedViewMaskCount = 0,
			.pCorrelatedViewMasks = nullptr
		};
		VkResult res = vkCreateRenderPass2(this->get_device().native(), &create, nullptr, &this->pass);

		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				hdk::error("Failed to create RenderPass because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				hdk::error("Failed to create RenderPass because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				hdk::error("Failed to create RenderPass but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	RenderPass::RenderPass(RenderPass&& move):
	pass(VK_NULL_HANDLE),
	info()
	{
		*this = std::move(move);
	}

	RenderPass::~RenderPass()
	{
		if(this->pass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(this->get_device().native(), this->pass, nullptr);
			this->pass = VK_NULL_HANDLE;
		}
	}

	RenderPass& RenderPass::operator=(RenderPass&& rhs)
	{
		std::swap(this->pass, rhs.pass);
		std::swap(this->info, rhs.info);
		return *this;
	}

	const LogicalDevice& RenderPass::get_device() const
	{
		hdk::assert(this->info.logical_device != nullptr, "RenderPassInfo contained nullptr LogicalDevice. Please submit a bug report.");
		return *this->info.logical_device;
	}

	const RenderPassInfo& RenderPass::get_info() const
	{
		return this->info;
	}

	RenderPass RenderPass::null()
	{
		return {};
	}

	bool RenderPass::is_null() const
	{
		return this->pass == VK_NULL_HANDLE;
	}

	RenderPass::NativeType RenderPass::native() const
	{
		return this->pass;
	}

	RenderPass::RenderPass():
	pass(VK_NULL_HANDLE),
	info(){}

	SubpassBuilder& SubpassBuilder::with_input_attachment(RenderPassInfo::InputAttachmentReference input_attachment)
	{
		this->subpass.input_attachments.add(input_attachment);
		return *this;
	}

	SubpassBuilder& SubpassBuilder::with_colour_attachment(RenderPassInfo::AttachmentReference colour_attachment)
	{
		this->subpass.colour_attachments.add(colour_attachment);
		return *this;
	}

	SubpassBuilder& SubpassBuilder::with_depth_stencil_attachment(RenderPassInfo::AttachmentReference depth_stencil_attachment)
	{
		this->subpass.depth_stencil_attachment = depth_stencil_attachment;
		return *this;
	}

	void SubpassBuilder::set_pipeline_context(PipelineContext context)
	{
		this->subpass.context = context;
	}

	const PipelineContext& SubpassBuilder::get_pipeline_context() const
	{
		return this->subpass.context;
	}

	const RenderPassInfo::Subpass& SubpassBuilder::build() const
	{
		return this->subpass;
	}

	RenderPassBuilder& RenderPassBuilder::with_attachment(Attachment attachment)
	{
		this->info.attachments.add(attachment);
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::with_subpass(RenderPassInfo::Subpass subpass)
	{
		this->info.subpasses.add(subpass);
		return *this;
	}

	const LogicalDevice* RenderPassBuilder::get_device() const
	{
		return this->info.logical_device;
	}

	void RenderPassBuilder::set_device(const LogicalDevice& device)
	{
		this->info.logical_device = &device;
	}

	RenderPass RenderPassBuilder::build() const
	{
		return {this->info};
	}
}

#endif // TZ_VULKAN
