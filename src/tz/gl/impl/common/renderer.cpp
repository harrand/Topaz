#include "tz/gl/impl/common/renderer.hpp"
#include "tz/gl/device.hpp"

namespace tz::gl
{

//--------------------------------------------------------------------------------------------------

	RendererEditBuilder& RendererEditBuilder::compute(renderer_edit::compute_config req)
	{
		this->request.push_back(req);
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::render_state(renderer_edit::render_config req)
	{
		this->request.push_back(req);
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::image_resize(renderer_edit::image_resize req)
	{
		this->request.push_back(req);
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::buffer_resize(renderer_edit::buffer_resize req)
	{
		this->request.push_back(req);
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::write(renderer_edit::resource_write req)
	{
		this->request.push_back(req);
		return *this;
	}

	renderer_edit_request RendererEditBuilder::build() const
	{
		return this->request;
	}

//--------------------------------------------------------------------------------------------------
	renderer_infoCommon::renderer_infoCommon()
	{
		#if HDK_DEBUG
			this->options |= tz::gl::renderer_option::NoPresent;
		#endif // HDK_DEBUG
	}

	unsigned int renderer_infoCommon::resource_count() const
	{
		return this->resources.size();
	}

	const iresource* renderer_infoCommon::get_resource(resource_handle handle)
	{
		return this->resources[static_cast<std::size_t>(static_cast<hdk::hanval>(handle))].get();
	}

	std::vector<const iresource*> renderer_infoCommon::get_resources() const
	{
		std::vector<const iresource*> ret;
		for(const auto& ptr : this->resources)
		{
			ret.push_back(ptr.get());
		}
		return ret;
	}

	std::span<const renderer_handle> renderer_infoCommon::get_dependencies() const
	{
		return this->dependencies;
	}

	std::span<const icomponent* const> renderer_infoCommon::get_components() const
	{
		return this->components;
	}

	resource_handle renderer_infoCommon::add_resource(const iresource& resource)
	{
		#if HDK_DEBUG
			if(resource.get_flags().contains(resource_flag::index_buffer))
			{
				hdk::assert(resource.get_type() == resource_type::buffer, "Attempting to add a resource with resource_flag::index_buffer specified, but the resource is not a buffer resource! Logic error/memory corruption? Please submit a bug report.");
				hdk::assert(!std::any_of(this->resources.begin(), this->resources.end(), [](const auto& r)->bool{return r != nullptr && r->get_flags().contains(resource_flag::index_buffer);}), "Attempting to add a resource with resource_flag::index_buffer specified, but a resource was already added which is an index buffer. You cannot have more than one index buffer in a renderer. Logic error? Please submit a bug report.");
			}
		#endif
		this->resources.push_back(resource.unique_clone());
		return static_cast<hdk::hanval>(this->real_resource_count() - 1);
	}

	resource_handle renderer_infoCommon::ref_resource(icomponent* component)
	{
		this->resources.push_back(nullptr);
		this->components.push_back(component);
		return static_cast<hdk::hanval>(this->real_resource_count() - 1);
	}

	resource_handle renderer_infoCommon::ref_resource(renderer_handle ren, resource_handle res)
	{
		return this->ref_resource(tz::gl::device().get_renderer(ren).get_component(res));
	}

	void renderer_infoCommon::set_output(const ioutput& output)
	{
		this->output = output.unique_clone();
	}

	const ioutput* renderer_infoCommon::get_output() const
	{
		return this->output.get();
	}

	const renderer_options& renderer_infoCommon::get_options() const
	{
		return this->options;
	}

	void renderer_infoCommon::set_options(renderer_options options)
	{
		#if HDK_DEBUG
			if(!options.contains(renderer_option::Internal_FinalDebugUIRenderer))
			{
				options |= renderer_option::NoPresent;
			}
		#endif
		this->options = options;
	}

	void renderer_infoCommon::add_dependency(renderer_handle dependency)
	{
		this->dependencies.push_back(dependency);
	}

	render_state& renderer_infoCommon::state()
	{
		return this->renderer_state;
	}

	const render_state& renderer_infoCommon::state() const
	{
		return this->renderer_state;
	}

	ShaderInfo& renderer_infoCommon::shader()
	{
		return this->shader_info;
	}

	const ShaderInfo& renderer_infoCommon::shader() const
	{
		return this->shader_info;
	}

	void renderer_infoCommon::debug_name(std::string debug_name)
	{
		this->dbg_name = debug_name;
	}

	std::string renderer_infoCommon::debug_get_name() const
	{
		#if HDK_DEBUG
			if(!this->dbg_name.empty())
			{
				return this->dbg_name;
			}
			std::size_t bufc = 0, imgc = 0;
			bufc = std::count_if(this->resources.begin(), this->resources.end(),
			[](const auto& res)
			{
				return res != nullptr && res->get_type() == resource_type::buffer;
			}) +
			std::count_if(this->components.begin(), this->components.end(),
			[](const icomponent* comp)
			{
				return comp != nullptr && comp->get_resource()->get_type() == resource_type::buffer;
			});
			imgc = std::count_if(this->resources.begin(), this->resources.end(),
			[](const auto& res)
			{
				return res != nullptr && res->get_type() == resource_type::image;
			}) +
			std::count_if(this->components.begin(), this->components.end(),
			[](const icomponent* comp)
			{
				return comp != nullptr && comp->get_resource()->get_type() == resource_type::image;
			});
			return (this->shader().has_shader(tz::gl::shader_stage::compute) ? "C" : "R") + std::to_string(bufc) + "b" + std::to_string(imgc) + std::string("i") + (this->output != nullptr && this->output->get_target() == output_target::offscreen_image ? "h" : "w");
		#else
			return "";
		#endif
	}

	std::size_t renderer_infoCommon::real_resource_count() const
	{
		return this->resources.size();
	}
}
