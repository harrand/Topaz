#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_RENDER_PASS_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_RENDER_PASS_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/logical_device.hpp"
#include "gl/impl/backend/vk2/image.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_render_pass
	 * Represents how some resource is loaded.
	 */
	enum class LoadOp
	{
		/// - Just load the data.
		Load = VK_ATTACHMENT_LOAD_OP_LOAD,
		/// - Clear the data to a known value.
		Clear = VK_ATTACHMENT_LOAD_OP_CLEAR,
		/// - Data is undefined.
		DontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_render_pass
	 * Represents how some resource is stored.
	 */
	enum class StoreOp
	{
		/// - Just store the data.
		Store = VK_ATTACHMENT_STORE_OP_STORE,
		/// - The resultant data is undefined.
		DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_render_pass
	 * Specifies the number of samples stored per image pixel.
	 */
	enum class SampleCount
	{
		One = VK_SAMPLE_COUNT_1_BIT,
		Two = VK_SAMPLE_COUNT_2_BIT,
		Four = VK_SAMPLE_COUNT_4_BIT,
		Eight = VK_SAMPLE_COUNT_8_BIT,
		Sixteen = VK_SAMPLE_COUNT_16_BIT,
		ThirtyTwo = VK_SAMPLE_COUNT_32_BIT,
		SixtyFour = VK_SAMPLE_COUNT_64_BIT
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_render_pass
	 * Specifies which pipeline type a RenderPass subpass is expected to bind to.
	 */
	enum class PipelineContext
	{
		Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
		Compute = VK_PIPELINE_BIND_POINT_COMPUTE,
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_render_pass
	 * Specifies which aspects of the image are included within a view.
	 */
	enum class ImageAspectFlag
	{
		Colour = VK_IMAGE_ASPECT_COLOR_BIT,
		Depth = VK_IMAGE_ASPECT_DEPTH_BIT,
		Stencil = VK_IMAGE_ASPECT_STENCIL_BIT
	};

	using ImageAspectFlags = tz::EnumField<ImageAspectFlag>;

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_render_pass
	 * Specifies some data about an attachment.
	 */
	struct Attachment
	{
		/// Specifies the format of the attachment. When used in a @ref RenderPass, its @ref LogicalDevice must support this format. See @ref PhysicalDevice::supports_image_colour_format
		ImageFormat format = ImageFormat::Undefined;
		/// Specifies the number of samples per pixel. See @ref SampleCount
		SampleCount sample_count = SampleCount::One;
		/// Specifies load operation for colour and depth when it is first used at the beginning of the subpass. See @ref LoadOp
		LoadOp colour_depth_load = LoadOp::Clear;
		/// Specifies store operation for colour and depth when it is last used at the end of the subpass. See @ref StoreOp
		StoreOp colour_depth_store = StoreOp::Store;
		/// Specifies load operation for stencil when it is first used at the beginning of the subpass. See @ref LoadOp
		LoadOp stencil_load = LoadOp::DontCare;
		/// Specifies store operation for stencil when it is last used at the end of the subpass. See @ref StoreOp
		StoreOp stencil_store = StoreOp::DontCare;
		/// Specifies what the layout the attached image will be in when render pass begins.
		ImageLayout initial_layout = ImageLayout::Undefined;
		/// Specifies what the layout the attached image will be in when the render pass ends.
		ImageLayout final_layout = ImageLayout::Undefined;

		bool operator==(const Attachment& rhs) const = default;
	};

	/**
	 * @ingroup tz_gl_vk_descriptors
	 * Specifies creation flags for a @ref RenderPass.
	 *
	 * See @ref RenderPassBuilder to help populate this structure.
	 */
	struct RenderPassInfo
	{
		/// Retrieve the total number of times an input attachment is referenced throughout all subpasses.
		std::size_t total_input_attachment_count() const;
		/// Retrieve the total number of times a colour attachment is refererenced throughout all subpasses.
		std::size_t total_colour_attachment_count() const;
		/// Query as to whether `this->logical_device` is a non-nullptr and non-null LogicalDevice which is suitable to create the resultant @ref RenderPass.
		bool has_valid_device() const;
		/// Iterate through the subpasses and ensure they reference attachments which exist. If this returns false, the resultant @ref RenderPass will almost certainly be invalid.
		bool values_make_sense() const;

		/**
		 * Structure which specifies a reference to an existing RenderPassInfo attachment, and the layout it should be in when referenced.
		 */
		struct AttachmentReference
		{
			/// Reference to the attachment such that we refer to `RenderPassInfo::attachments[attachment_idx]. This must be less than the total number of attachments.`
			std::uint32_t attachment_idx;
			/// Specifies the layout which the attachment will be in during the subpass.
			ImageLayout current_layout;
		};

		/**
		 * A specialised @ref AttachmentReference which is suitable to reference input attachments.
		 */
		struct InputAttachmentReference
		{
			AttachmentReference attachment_ref;
			ImageAspectFlags aspect_flags;
		};

		/**
		 * Specifies information about a RenderPass subpass.
		 *
		 * See @ref SubpassBuilder to help populate this structure.
		 */
		struct Subpass
		{
			/// Specifies how this subpass binds to the graphics pipeline.
			PipelineContext context = PipelineContext::Graphics;
			/// List of all attachments which will be input attachments during this subpass.
			tz::BasicList<InputAttachmentReference> input_attachments;
			/// List of all attachments which will be colour attachments during this subpass.
			tz::BasicList<AttachmentReference> colour_attachments;
			/// Optional reference to an attachment which will be the depth-stencil attachment during this subpass.
			std::optional<AttachmentReference> depth_stencil_attachment = std::nullopt;
		};

		/// List of all attachments. These attachments are referenced throughout the subpasses.
		tz::BasicList<Attachment> attachments;
		/// List of all subpasses, in order.
		tz::BasicList<Subpass> subpasses;
		/// @ref LogicalDevice which will be used to create the @ref RenderPass. It must point to a valid device.
		const LogicalDevice* logical_device = nullptr;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_render_pass
	 * Represents a collection of attachments and subpasses and describes how the attachments are used throughout the subpases.
	 */
	class RenderPass
	{
	public:
		/**
		 * Create the RenderPass.
		 */
		RenderPass(RenderPassInfo info);
		RenderPass(const RenderPass& copy) = delete;
		RenderPass(RenderPass&& move);
		~RenderPass();

		RenderPass& operator=(const RenderPass& rhs) = delete;
		RenderPass& operator=(RenderPass&& rhs);

		const LogicalDevice& get_device() const;

		using NativeType = VkRenderPass;
		NativeType native() const;
	private:
		VkRenderPass pass;
		RenderPassInfo info;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_render_pass
	 * Helper class to create a @ref RenderPassInfo::Subpass.
	 */
	class SubpassBuilder
	{
	public:
		SubpassBuilder() = default;
		/**
		 * Have the subpass reference an existing attachment as an input attachment.
		 * @param input_attachment Information about which attachment will be referenced as an input attachment.
		 */
		SubpassBuilder& with_input_attachment(RenderPassInfo::InputAttachmentReference input_attachment);
		/**
		 * Have the subpass reference an existing attachment as a colour attachment.
		 * @param colour_attachment Information about which attachment will be referenced as a colour attachment.
		 */
		SubpassBuilder& with_colour_attachment(RenderPassInfo::AttachmentReference colour_attachment);
		/**
		 * Have the subpass reference an existing attachment as the depth-stencil attachment. Note that only one attachment can be referenced as a depth-stencil attachment at a time - If this is called multiple times for the same subpass, only the newest invocation will apply.
		 * @param depth_stencil_attachment Information about which attachment will be referenced as a depth-stencil attachment.
		 */
		SubpassBuilder& with_depth_stencil_attachment(RenderPassInfo::AttachmentReference depth_stencil_attachment);

		/**
		 * Specify how the subpass binds to the graphics pipeline.
		 * @param context Bind point to the graphics pipeline.
		 */
		void set_pipeline_context(PipelineContext context);
		/**
		 * Query what the current bind point to the graphics pipeline is. By default, this will be @ref PipelineContext::Graphics.
		 */
		const PipelineContext& get_pipeline_context() const;
		/**
		 * Return a subpass info structure based upon this builder.
		 * @return New Subpass info.
		 */
		const RenderPassInfo::Subpass& build() const;
	private:
		RenderPassInfo::Subpass subpass;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_render_pass
	 * Helper class to create a @ref RenderPass.
	 */
	class RenderPassBuilder
	{
	public:
		RenderPassBuilder() = default;
		/**
		 * Add a new attachment to the pass. This can be referenced by the subpasses.
		 * @param attachment Information about the attachment and how it is interpreted during subpasses.
		 */
		RenderPassBuilder& with_attachment(Attachment attachment);
		/**
		 * Add a new subpass to the render pass. This can reference any existing attachments.
		 * @param subpass Information about the subpass and how it references existing attachments.
		 */
		RenderPassBuilder& with_subpass(RenderPassInfo::Subpass subpass);

		/**
		 * Retrieve the @ref LogicalDevice which will be used to construct the @ref RenderPass. By default this is nullptr.
		 * @return Current LogicalDevice, or nullptr if no device was ever set.
		 */
		const LogicalDevice* get_device() const;
		/**
		 * Set which @ref LogicalDevice will be used to construct the @ref RenderPass.
		 * @param device LogicalDevice which will be used. It must not be a null device.
		 */
		void set_device(const LogicalDevice& device);

		/**
		 * Create a new @ref RenderPass based upon this builder.
		 * @return New RenderPass.
		 */
		RenderPass build() const;
	private:
		RenderPassInfo info;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_RENDER_PASS_HPP
