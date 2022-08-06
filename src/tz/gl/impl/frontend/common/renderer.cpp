#include "tz/gl/impl/frontend/common/renderer.hpp"

namespace tz::gl
{

//--------------------------------------------------------------------------------------------------

	RendererEditBuilder& RendererEditBuilder::compute(RendererComputeEditRequest req)
	{
		this->request.compute_edit = req;
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::render_state(RendererStateEditRequest req)
	{
		this->request.render_state_edit = req;
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::image_resize(RendererImageComponentResizeRequest req)
	{
		this->request.component_edits.push_back(req);
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::buffer_resize(RendererBufferComponentResizeRequest req)
	{
		this->request.component_edits.push_back(req);
		return *this;
	}

	RendererEditBuilder& RendererEditBuilder::write(RendererComponentWriteRequest req)
	{
		this->request.component_edits.push_back(req);
		return *this;
	}

	RendererEditRequest RendererEditBuilder::build() const
	{
		return this->request;
	}

//--------------------------------------------------------------------------------------------------
	unsigned int RendererInfoCommon::resource_count() const
	{
		return this->resources.size();
	}

	const IResource* RendererInfoCommon::get_resource(ResourceHandle handle)
	{
		return this->resources[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))];
	}

	std::span<const IResource* const> RendererInfoCommon::get_resources() const
	{
		return this->resources;
	}

	std::span<const IComponent* const> RendererInfoCommon::get_components() const
	{
		return this->components;
	}

	ResourceHandle RendererInfoCommon::add_resource(IResource& resource)
	{
		#if TZ_DEBUG
			if(resource.get_flags().contains(ResourceFlag::IndexBuffer))
			{
				tz_assert(resource.get_type() == ResourceType::Buffer, "Attempting to add a resource with ResourceFlag::IndexBuffer specified, but the resource is not a buffer resource! Logic error/memory corruption? Please submit a bug report.");
				tz_assert(!std::any_of(this->resources.begin(), this->resources.end(), [](const IResource* r)->bool{return r != nullptr && r->get_flags().contains(ResourceFlag::IndexBuffer);}), "Attempting to add a resource with ResourceFlag::IndexBuffer specified, but a resource was already added which is an index buffer. You cannot have more than one index buffer in a renderer. Logic error? Please submit a bug report.");
			}
		#endif
		this->resources.push_back(&resource);
		return static_cast<tz::HandleValue>(this->real_resource_count() - 1);
	}

	ResourceHandle RendererInfoCommon::add_component(IComponent& component)
	{
		this->resources.push_back(nullptr);
		this->components.push_back(&component);
		return static_cast<tz::HandleValue>(this->real_resource_count() - 1);
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
		this->options = options;
	}

	void RendererInfoCommon::set_clear_colour(tz::Vec4 clear_colour)
	{
		this->clear_colour = clear_colour;
	}

	const tz::Vec4& RendererInfoCommon::get_clear_colour() const
	{
		return this->clear_colour;
	}

	void RendererInfoCommon::set_compute_kernel(tz::Vec3ui compute_kernel)
	{
		this->compute_kernel = compute_kernel;
	}

	const tz::Vec3ui& RendererInfoCommon::get_compute_kernel() const
	{
		return this->compute_kernel;
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
		#if TZ_DEBUG
			if(!this->dbg_name.empty())
			{
				return this->dbg_name;
			}
			std::size_t bufc = 0, imgc = 0;
			bufc = std::count_if(this->resources.begin(), this->resources.end(),
			[](const IResource* res)
			{
				return res != nullptr && res->get_type() == ResourceType::Buffer;
			}) +
			std::count_if(this->components.begin(), this->components.end(),
			[](const IComponent* comp)
			{
				return comp != nullptr && comp->get_resource()->get_type() == ResourceType::Buffer;
			});
			imgc = std::count_if(this->resources.begin(), this->resources.end(),
			[](const IResource* res)
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