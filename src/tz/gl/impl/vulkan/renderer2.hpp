#ifndef TZ_GL_IMPL_VULKAN_RENDERER2_HPP
#define TZ_GL_IMPL_VULKAN_RENDERER2_HPP
#include "tz/gl/api/renderer.hpp"
#include "tz/core/memory/maybe_owned_ptr.hpp"
#include "tz/gl/impl/vulkan/component.hpp"
#include "tz/gl/impl/common/renderer.hpp"

#include "tz/gl/impl/vulkan/detail/descriptors.hpp"

namespace tz::gl
{
	// to encapsulate the whole state of a vulkan renderer leads to *alot* of code.
	// to make this alot more palatable, each "major feature" of a vulkan renderer is its own class, and they form an inheritance branch leading to the fully-featured renderer_vulkan2.
	// i.e renderer_resource_manager -> renderer_descriptor_manager -> ... -> renderer_vulkan2
	// each of these "major feature" classes are unaware of the functionality of the next-level in the chain. this means that the classes get more and more access to things.
	// note: this also means that the implementation of renderer_type is done piecemeal - as we use public inheritance, eventually renderer_vulkan2 will inherit all the api impls from its lower-level components. 

	// represents topaz-level resource management.
	// the majority of this code is not specific to vulkan, however setting up dynamic-resource-spans is.
	// possible todo: bring most of this class out into a common impl for opengl? 
	class renderer_resource_manager : private AssetStorageCommon<iresource>
	{
	public:
		renderer_resource_manager(const tz::gl::renderer_info& rinfo);
		renderer_resource_manager() = default;

		unsigned int resource_count() const;
		const iresource* get_resource(tz::gl::resource_handle rh) const;
		iresource* get_resource(tz::gl::resource_handle rh);
		const icomponent* get_component(tz::gl::resource_handle rh) const;
		icomponent* get_component(tz::gl::resource_handle rh);
	private:
		void patch_resource_references(const tz::gl::renderer_info& rinfo);
		void setup_dynamic_resource_spans();
		tz::maybe_owned_ptr<icomponent> make_component_from(iresource* resource);
		std::vector<tz::maybe_owned_ptr<icomponent>> components = {};
	};

	// responsible for making sure our topaz-level resources are represented as descriptor sets correctly.
	class renderer_descriptor_manager : public renderer_resource_manager
	{
	public:
		renderer_descriptor_manager(const tz::gl::renderer_info& rinfo);
		renderer_descriptor_manager() = default;
		// descriptor manager is empty if there are no descriptors to bind.
		bool empty() const;
	private:
		void deduce_descriptor_layout(const tz::gl::render_state& state);
		void collect_descriptors();
		vk2::DescriptorLayout layout = vk2::DescriptorLayout::null();
		vk2::DescriptorPool::AllocationResult descriptors = {};
	};

	class renderer_vulkan2 : public renderer_descriptor_manager
	{
	public:
		renderer_vulkan2(const tz::gl::renderer_info& rinfo);
		// Satisfies tz::nullable
		static renderer_vulkan2 null();
		bool is_null() const;

		// NYI
		const ioutput* get_output() const {return nullptr;}
		const tz::gl::renderer_options& get_options() const;
		const tz::gl::render_state& get_state() const;
		// NYI
		void render() {}
		// NYI
		void edit(tz::gl::renderer_edit_request req){(void)req;}
		// NYI
		void dbgui(){}
		// NYI
		std::string_view debug_get_name(){return "NYI";}

	private:
		renderer_vulkan2() = default;

		tz::gl::renderer_options options = {};
		tz::gl::render_state state = {};
	};
	static_assert(tz::gl::renderer_type<renderer_vulkan2>);
}

#endif // TZ_GL_IMPL_VULKAN_RENDERER2_HPP
