#ifndef TZ_GL_IMPL_VULKAN_RENDERER2_HPP
#define TZ_GL_IMPL_VULKAN_RENDERER2_HPP
#include "tz/gl/api/renderer.hpp"
#include "tz/core/memory/maybe_owned_ptr.hpp"
#include "tz/gl/impl/vulkan/component.hpp"
#include "tz/gl/impl/common/renderer.hpp"
#include <functional>

#include "tz/gl/impl/vulkan/detail/descriptors.hpp"
#include "tz/gl/impl/vulkan/detail/command.hpp"
#include "tz/gl/impl/vulkan/detail/fence.hpp"
#include "tz/gl/impl/vulkan/detail/semaphore.hpp"

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
		renderer_resource_manager(const renderer_resource_manager& copy) = delete;
		renderer_resource_manager(renderer_resource_manager&& move) = default;
		renderer_resource_manager() = default;
		renderer_resource_manager& operator=(const renderer_resource_manager& copy) = delete;
		renderer_resource_manager& operator=(renderer_resource_manager&& move) = default;

		unsigned int resource_count() const;
		const iresource* get_resource(tz::gl::resource_handle rh) const;
		iresource* get_resource(tz::gl::resource_handle rh);
		const icomponent* get_component(tz::gl::resource_handle rh) const;
		icomponent* get_component(tz::gl::resource_handle rh);
	protected:
		std::span<vk2::ImageView> get_image_resource_views();
		std::span<vk2::Sampler> get_image_resource_samplers();
		void notify_image_dirty(tz::gl::resource_handle rh);
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
		renderer_descriptor_manager(const renderer_descriptor_manager& copy) = delete;
		renderer_descriptor_manager(renderer_descriptor_manager&& move) = default;
		renderer_descriptor_manager() = default;
		renderer_descriptor_manager& operator=(const renderer_descriptor_manager& copy) = delete;
		renderer_descriptor_manager& operator=(renderer_descriptor_manager&& move) = default;
	protected:
		const vk2::DescriptorLayout& get_descriptor_layout() const;
		std::span<const vk2::DescriptorSet> get_descriptor_sets() const;
		// descriptor manager is empty if there are no descriptors to bind.
		bool empty() const;
		void write_descriptors(const tz::gl::render_state& state);
	private:
		void deduce_descriptor_layout(const tz::gl::render_state& state);
		void allocate_descriptors();
		vk2::DescriptorData descriptors = {};
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
		const ioutput* get_output() const;
	protected:
		std::span<render_target_t> get_render_targets();
		tz::vec2ui get_render_target_dimensions() const;
		bool targets_window() const;
		void populate_render_targets();
	private:
		std::unique_ptr<tz::gl::ioutput> output = nullptr;
		std::vector<render_target_t> render_targets = {};
	};

	class renderer_pipeline : public renderer_output_manager
	{
	public:
		renderer_pipeline(const tz::gl::renderer_info& rinfo);
		renderer_pipeline(const renderer_pipeline& copy) = delete;
		renderer_pipeline(renderer_pipeline&& move) = default;
		renderer_pipeline() = default;
		renderer_pipeline& operator=(const renderer_pipeline& copy) = delete;
		renderer_pipeline& operator=(renderer_pipeline&& move) = default;
		enum class pipeline_type_t
		{
			graphics,
			compute
		};
	protected:
		pipeline_type_t get_pipeline_type() const;
		const vk2::Pipeline& get_pipeline() const;
		const vk2::PipelineLayout& get_pipeline_layout() const;
		void update_pipeline();
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
		vk2::Shader shader = vk2::Shader::null();
		vk2::PipelineData pipeline = {};
		// depends purely on renderer options so this should never change.
		pipeline_invariant_config_t pipeline_config = pipeline_invariant_config_t::null();
	};

	class renderer_command_processor : public renderer_pipeline
	{
	public:
		renderer_command_processor(const tz::gl::renderer_info& info);
		renderer_command_processor(const renderer_command_processor& copy) = delete;
		renderer_command_processor(renderer_command_processor&& move) = default;
		// potential bug: command buffers retrieved from a move may have pool owner reference that is invalid?
		~renderer_command_processor();
		renderer_command_processor& operator=(const renderer_command_processor& copy) = delete;
		renderer_command_processor& operator=(renderer_command_processor&& move) = default;
		renderer_command_processor() = default;

		enum class command_type
		{
			work,
			scratch,
			both
		};
	protected:
		void do_scratch_work(std::function<void(vk2::CommandBufferRecording&)> record_commands);
		void do_frame();
		void set_work_commands(std::function<void(vk2::CommandBufferRecording&, unsigned int)> work_record_commands);
		void record_commands(const tz::gl::render_state& state, const tz::gl::renderer_options& options, std::string label);
		void scratch_initialise_static_resources();
	private:
		void record_render_commands(const tz::gl::render_state& state, const tz::gl::renderer_options& options, std::string label);
		void record_compute_commands(const tz::gl::render_state& state, const tz::gl::renderer_options& options, std::string label);
		void allocate_commands(command_type t = command_type::both);
		void free_commands(command_type t = command_type::both);
		void do_static_resource_transfers(std::span<vk2::Buffer> resource_staging_buffers);

		std::span<vk2::CommandBuffer> work_command_buffers();
		vk2::CommandBuffer& scratch_command_buffer();
		bool render_wait_enabled = false;
		bool no_present_enabled = false;
		std::vector<vk2::CommandPool::AllocationResult> command_allocations = {};
		vk2::Fence render_wait_fence = vk2::Fence::null();
		vk2::BinarySemaphore present_sync_semaphore = vk2::BinarySemaphore::null();
	};

	class renderer_vulkan2 : public renderer_command_processor
	{
	public:
		renderer_vulkan2(const tz::gl::renderer_info& rinfo);
		renderer_vulkan2(const renderer_vulkan2& copy) = delete;
		renderer_vulkan2(renderer_vulkan2&& move) = default;
		~renderer_vulkan2();
		renderer_vulkan2& operator=(const renderer_vulkan2& copy) = delete;
		renderer_vulkan2& operator=(renderer_vulkan2&& move) = default;
		// NYI
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
		void check_and_handle_resize();
		void do_resize();

		tz::gl::renderer_options options = {};
		tz::gl::render_state state = {};
		tz::vec2ui window_cache_dims = {};
		bool null_flag = true;
	};
	static_assert(tz::gl::renderer_type<renderer_vulkan2>);
}

#endif // TZ_GL_IMPL_VULKAN_RENDERER2_HPP
