#ifndef TZ_GL_IMPL_VULKAN_RENDERER2_HPP
#define TZ_GL_IMPL_VULKAN_RENDERER2_HPP
#include "tz/gl/api/renderer.hpp"
#include "tz/core/memory/maybe_owned_ptr.hpp"
#include "tz/gl/impl/vulkan/component.hpp"
#include "tz/gl/impl/common/renderer.hpp"

namespace tz::gl
{
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
		tz::maybe_owned_ptr<icomponent> make_component_from(iresource* resource);
		std::vector<tz::maybe_owned_ptr<icomponent>> components = {};
	};

	class renderer_vulkan2 : public renderer_resource_manager
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
