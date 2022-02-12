#include "gl/impl/frontend/common/renderer.hpp"

namespace tz::gl2
{

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

	ResourceHandle RendererInfoCommon::add_resource(IResource& resource)
	{
		this->resources.push_back(&resource);
		return static_cast<tz::HandleValue>(this->resources.size() - 1);
	}

	void RendererInfoCommon::set_output(IOutput& output)
	{
		this->output = &output;
	}

	const IOutput* RendererInfoCommon::get_output() const
	{
		return this->output;
	}

	IOutput* RendererInfoCommon::get_output()
	{
		return this->output;
	}

	const RendererOptions& RendererInfoCommon::get_options() const
	{
		return this->options;
	}

	void RendererInfoCommon::set_options(RendererOptions options)
	{
		this->options = options;
	}

	ShaderInfo& RendererInfoCommon::shader()
	{
		return this->shader_info;
	}

	const ShaderInfo& RendererInfoCommon::shader() const
	{
		return this->shader_info;
	}

}
