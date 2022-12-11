#include "tz/gl/impl/frontend/common/renderer.hpp"
#include "tz/gl/device.hpp"

namespace tz::gl
{

//--------------------------------------------------------------------------------------------------

	RendererEditBuilder& RendererEditBuilder::compute(RendererEdit::ComputeConfig req)
	{
		this->request.push_back(req);
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::render_state(RendererEdit::RenderConfig req)
	{
		this->request.push_back(req);
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::image_resize(RendererEdit::ImageResize req)
	{
		this->request.push_back(req);
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::buffer_resize(RendererEdit::BufferResize req)
	{
		this->request.push_back(req);
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::write(RendererEdit::ResourceWrite req)
	{
		this->request.push_back(req);
		return *this;
	}

	RendererEditRequest RendererEditBuilder::build() const
	{
		return this->request;
	}

//--------------------------------------------------------------------------------------------------
	RendererInfoCommon::RendererInfoCommon()
	{
		#if HDK_DEBUG
			this->options |= tz::gl::RendererOption::NoPresent;
		#endif // HDK_DEBUG
	}

	unsigned int RendererInfoCommon::resource_count() const
	{
		return this->resources.size();
	}

	const IResource* RendererInfoCommon::get_resource(ResourceHandle handle)
	{
		return this->resources[static_cast<std::size_t>(static_cast<hdk::hanval>(handle))].get();
	}

	std::vector<const IResource*> RendererInfoCommon::get_resources() const
	{
		std::vector<const IResource*> ret;
		for(const auto& ptr : this->resources)
		{
			ret.push_back(ptr.get());
		}
		return ret;
	}

	std::span<const RendererHandle> RendererInfoCommon::get_dependencies() const
	{
		return this->dependencies;
	}

	std::span<const IComponent* const> RendererInfoCommon::get_components() const
	{
		return this->components;
	}

	ResourceHandle RendererInfoCommon::add_resource(const IResource& resource)
	{
		#if HDK_DEBUG
			if(resource.get_flags().contains(ResourceFlag::IndexBuffer))
			{
				hdk::assert(resource.get_type() == ResourceType::Buffer, "Attempting to add a resource with ResourceFlag::IndexBuffer specified, but the resource is not a buffer resource! Logic error/memory corruption? Please submit a bug report.");
				hdk::assert(!std::any_of(this->resources.begin(), this->resources.end(), [](const auto& r)->bool{return r != nullptr && r->get_flags().contains(ResourceFlag::IndexBuffer);}), "Attempting to add a resource with ResourceFlag::IndexBuffer specified, but a resource was already added which is an index buffer. You cannot have more than one index buffer in a renderer. Logic error? Please submit a bug report.");
			}
		#endif
		this->resources.push_back(resource.unique_clone());
		return static_cast<hdk::hanval>(this->real_resource_count() - 1);
	}

	ResourceHandle RendererInfoCommon::ref_resource(IComponent* component)
	{
		this->resources.push_back(nullptr);
		this->components.push_back(component);
		return static_cast<hdk::hanval>(this->real_resource_count() - 1);
	}

	ResourceHandle RendererInfoCommon::ref_resource(RendererHandle ren, ResourceHandle res)
	{
		return this->ref_resource(tz::gl::device().get_renderer(ren).get_component(res));
	}

	void RendererInfoCommon::set_output(const IOutput& output)
	{
		this->output = output.unique_clone();
	}

	const IOutput* RendererInfoCommon::get_output() const
	{
		return this->output.get();
	}

	const RendererOptions& RendererInfoCommon::get_options() const
	{
		return this->options;
	}

	void RendererInfoCommon::set_options(RendererOptions options)
	{
		#if HDK_DEBUG
			if(!options.contains(RendererOption::Internal_FinalDebugUIRenderer))
			{
				options |= RendererOption::NoPresent;
			}
		#endif
		this->options = options;
	}

	void RendererInfoCommon::add_dependency(RendererHandle dependency)
	{
		this->dependencies.push_back(dependency);
	}

	RenderState& RendererInfoCommon::state()
	{
		return this->renderer_state;
	}

	const RenderState& RendererInfoCommon::state() const
	{
		return this->renderer_state;
	}

	ShaderInfo& RendererInfoCommon::shader()
	{
		return this->shader_info;
	}

	const ShaderInfo& RendererInfoCommon::shader() const
	{
		return this->shader_info;
	}

	void RendererInfoCommon::debug_name(std::string debug_name)
	{
		this->dbg_name = debug_name;
	}

	std::string RendererInfoCommon::debug_get_name() const
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
				return res != nullptr && res->get_type() == ResourceType::Buffer;
			}) +
			std::count_if(this->components.begin(), this->components.end(),
			[](const IComponent* comp)
			{
				return comp != nullptr && comp->get_resource()->get_type() == ResourceType::Buffer;
			});
			imgc = std::count_if(this->resources.begin(), this->resources.end(),
			[](const auto& res)
			{
				return res != nullptr && res->get_type() == ResourceType::Image;
			}) +
			std::count_if(this->components.begin(), this->components.end(),
			[](const IComponent* comp)
			{
				return comp != nullptr && comp->get_resource()->get_type() == ResourceType::Image;
			});
			return (this->shader().has_shader(tz::gl::ShaderStage::Compute) ? "C" : "R") + std::to_string(bufc) + "b" + std::to_string(imgc) + std::string("i") + (this->output != nullptr && this->output->get_target() == OutputTarget::OffscreenImage ? "h" : "w");
		#else
			return "";
		#endif
	}

	std::size_t RendererInfoCommon::real_resource_count() const
	{
		return this->resources.size();
	}
}
