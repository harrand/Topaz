#ifndef TZ_GL_IMPL_VULKAN_RENDERER2_HPP
#define TZ_GL_IMPL_VULKAN_RENDERER2_HPP
#include "tz/gl/api/renderer.hpp"
#include "tz/core/memory/maybe_owned_ptr.hpp"
#include "tz/gl/impl/vulkan/component.hpp"
#include "tz/gl/impl/common/renderer.hpp"
#include <functional>

#include "tz/gl/impl/vulkan/detail/descriptors.hpp"
#include "tz/gl/impl/vulkan/detail/command.hpp"

namespace tz::gl
{
	// to encapsulate the whole state of a vulkan renderer leads to *alot* of code.
	// to make this alot more palatable, each "major feature" of a vulkan renderer is its own class, and they form an inheritance branch leading to the fully-featured renderer_vulkan2.
	// i.e renderer_resource_manager -> renderer_descriptor_manager -> ... -> renderer_vulkan2
	// each of these "major feature" classes are unaware of the functionality of the next-level in the chain. this means that the classes get more and more access to things.
	// note: this also means that the implementation of renderer_type is done piecemeal - as we use public inheritance, eventually renderer_vulkan2 will inherit all the api impls from its lower-level components. 

	struct renderer_vulkan_base
	{
		protected:
		// devices have this concept of renderer handles, but they are not guaranteed to be unique (e.g if renderer handle 2 is deleted and a new renderer is created, that will also have handle 2.)
		// this is a uid which will uniquely identify ths current renderer. renderers need to have their own identity because other manager classes (mainly device_vulkan2) does bookkeeping for renderers and needs to know who is who.
		static unsigned int uid_counter;
		unsigned int uid = uid_counter++;
	public:
		unsigned int vk_get_uid() const{return this->uid;}
	};

	// represents topaz-level resource management.
	// the majority of this code is not specific to vulkan, however setting up dynamic-resource-spans is.
	// possible todo: bring most of this class out into a common impl for opengl? 
	class renderer_resource_manager : private AssetStorageCommon<iresource>, public renderer_vulkan_base
	{
	public:
		renderer_resource_manager(const tz::gl::renderer_info& rinfo);
		renderer_resource_manager() = default;

		unsigned int resource_count() const;
		const iresource* get_resource(tz::gl::resource_handle rh) const;
		iresource* get_resource(tz::gl::resource_handle rh);
		const icomponent* get_component(tz::gl::resource_handle rh) const;
		icomponent* get_component(tz::gl::resource_handle rh);
	protected:
		std::span<vk2::ImageView> get_image_resource_views();
		std::span<vk2::Sampler> get_image_resource_samplers();
	private:
		void patch_resource_references(const tz::gl::renderer_info& rinfo);
		void setup_dynamic_resource_spans();
		void populate_image_resource_views();
		void populate_image_resource_samplers();
		static vk2::SamplerInfo make_fitting_sampler(const iresource& res);
		tz::maybe_owned_ptr<icomponent> make_component_from(iresource* resource);
		std::vector<tz::maybe_owned_ptr<icomponent>> components = {};
		// one entry per component - symmetrical to this->components. if component is a buffer though, the view is vk2::ImageView::null()
		std::vector<vk2::ImageView> image_resource_views = {};
		std::vector<vk2::Sampler> image_resource_samplers = {};
	};

	// responsible for making sure our topaz-level resources are represented as descriptor sets correctly.
	class renderer_descriptor_manager : public renderer_resource_manager
	{
	public:
		renderer_descriptor_manager(const tz::gl::renderer_info& rinfo);
		renderer_descriptor_manager() = default;
	protected:
		const vk2::DescriptorLayout& get_descriptor_layout() const;
		std::span<const vk2::DescriptorSet> get_descriptor_sets() const;
		// descriptor manager is empty if there are no descriptors to bind.
		bool empty() const;
	private:
		void deduce_descriptor_layout(const tz::gl::render_state& state);
		void allocate_descriptors();
		void write_descriptors(const tz::gl::render_state& state);
		vk2::DescriptorLayout layout = vk2::DescriptorLayout::null();
		vk2::DescriptorPool::AllocationResult descriptors = {};
	};

	class renderer_output_manager : public renderer_descriptor_manager
	{
	public:
		renderer_output_manager(const tz::gl::renderer_info& rinfo);
		renderer_output_manager() = default;

		struct render_target_t
		{
			std::vector<vk2::ImageView> colour_attachments = {};
			vk2::ImageView depth_attachment = vk2::ImageView::null();
		};
	protected:
		std::span<render_target_t> get_render_targets();
		tz::vec2ui get_render_target_dimensions() const;
	private:
		void populate_render_targets();

		const tz::gl::ioutput* output = nullptr;
		std::vector<render_target_t> render_targets = {};
	};

	class renderer_pipeline : public renderer_output_manager
	{
	public:
		renderer_pipeline(const tz::gl::renderer_info& rinfo);
		renderer_pipeline() = default;
		enum class pipeline_type_t
		{
			graphics,
			compute
		};
	protected:
		pipeline_type_t get_pipeline_type() const;
		const vk2::Pipeline& get_pipeline() const;
		const vk2::PipelineLayout& get_pipeline_layout() const;
	private:
		struct pipeline_invariant_config_t
		{
			bool depth_testing = false;
			bool alpha_blending = false;
			pipeline_type_t type = pipeline_type_t::graphics;
			// all of these cannot change for a renderer. wireframe mode can be disabled/enabled, so we don't want to cache that. have to query that every time.
			bool valid = true;
			static pipeline_invariant_config_t null(){return {.valid = false};}
		};
		void deduce_pipeline_config(const tz::gl::renderer_info& rinfo);
		void deduce_pipeline_layout();
		void create_shader(const tz::gl::renderer_info& rinfo);
		void update_pipeline();
		vk2::Shader shader = vk2::Shader::null();
		vk2::PipelineLayout pipeline_layout = vk2::PipelineLayout::null();
		vk2::Pipeline pipeline = vk2::Pipeline::null();
		// depends purely on renderer options so this should never change.
		pipeline_invariant_config_t pipeline_config = pipeline_invariant_config_t::null();
	};

	class renderer_command_processor : public renderer_pipeline
	{
	public:
		renderer_command_processor(const tz::gl::renderer_info& info);
		~renderer_command_processor();
		renderer_command_processor() = default;

		enum class command_type
		{
			work,
			scratch,
			both
		};
	protected:
		void do_scratch_work(std::function<void(vk2::CommandBufferRecording&)> record_commands);
		void set_work_commands(std::function<void(vk2::CommandBufferRecording&, unsigned int)> work_record_commands);
		void record_render_commands(const tz::gl::render_state& state, std::string label);
		void record_compute_commands(const tz::gl::render_state& state, std::string label);
	private:
		void allocate_commands(command_type t = command_type::both);
		void free_commands(command_type t = command_type::both);
		void scratch_initialise_static_resources();
		void do_static_resource_transfers(std::span<vk2::Buffer> resource_staging_buffers);

		std::span<vk2::CommandBuffer> work_command_buffers();
		vk2::CommandBuffer& scratch_command_buffer();
		bool render_wait_enabled = false;
		std::vector<vk2::CommandPool::AllocationResult> command_allocations = {};
	};

	class renderer_vulkan2 : public renderer_command_processor
	{
	public:
		renderer_vulkan2(const tz::gl::renderer_info& rinfo);
		// NYI
		const ioutput* get_output() const;
		const tz::gl::renderer_options& get_options() const;
		const tz::gl::render_state& get_state() const;
		void render();
		void edit(tz::gl::renderer_edit_request req);
		void dbgui();
		// NYI
		std::string_view debug_get_name() const;

		// Satisfies tz::nullable
		static renderer_vulkan2 null();
		bool is_null() const;
	private:
		renderer_vulkan2() = default;

		tz::gl::renderer_options options = {};
		tz::gl::render_state state = {};
	};
	static_assert(tz::gl::renderer_type<renderer_vulkan2>);
}

#endif // TZ_GL_IMPL_VULKAN_RENDERER2_HPP