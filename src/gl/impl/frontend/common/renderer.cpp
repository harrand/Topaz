#include "gl/impl/frontend/common/renderer.hpp"

namespace tz::gl
{
	RendererInputHandle RendererBuilderBase::add_input(const IRendererInput& input)
	{
		auto sz = this->inputs.size();
		this->inputs.push_back(&input);
		return static_cast<tz::HandleValue>(sz);
	}

	const IRendererInput* RendererBuilderBase::get_input(RendererInputHandle handle) const
	{
		std::size_t input_id = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
		tz_assert(input_id < this->inputs.size(), "Handle %zu is invalid for this RendererInput. Perhaps this input belongs to another Renderer?", input_id);
		return this->inputs[input_id];
	}

	std::size_t RendererBuilderBase::input_count() const
	{
		return this->inputs.size();
	}

	void RendererBuilderBase::set_output(IRendererOutput& output)
	{
		this->output = &output;
	}

	const IRendererOutput* RendererBuilderBase::get_output() const
	{
		return this->output;
	}

	IRendererOutput* RendererBuilderBase::get_output()
	{
		return this->output;
	}

	ResourceHandle RendererBuilderBase::add_resource(const IResource& resource)
	{
		std::size_t total_resource_size = this->buffer_resources.size() + this->texture_resources.size();
		switch(resource.get_type())
		{
			case ResourceType::Buffer:
				this->buffer_resources.push_back(&resource);
				return {static_cast<tz::HandleValue>(total_resource_size)};
			break;
			case ResourceType::Texture:
				this->texture_resources.push_back(&resource);
				return {static_cast<tz::HandleValue>(total_resource_size)};
			break;
			default:
				tz_error("Unexpected resource type. Support for this resource type is not yet implemented (Vulkan)");
				return {static_cast<tz::HandleValue>(0)};
			break;
		}
	}

	const IResource* RendererBuilderBase::get_resource(ResourceHandle handle) const
	{
		auto handle_value = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
		if(handle_value >= this->buffer_resources.size())
		{
			// Handle value doesn't fit within buffer resources, must be a texture resource or invalid.
			if(handle_value < this->buffer_resources.size() + this->texture_resources.size())
			{
				// Is within range, we assume it's a valid texture resource
				return this->texture_resources[handle_value - this->buffer_resources.size()];
			}
			else
			{
				// Invalid, probably someone else's ResourceHsndle
				return nullptr;
			}
		}
		// Is within range, we assume it's a valid buffer resource
		return this->buffer_resources[handle_value];
	}

	std::span<const IResource* const> RendererBuilderBase::get_resources(ResourceType type) const
	{
		switch(type)
		{
			case ResourceType::Buffer:

				return {this->buffer_resources.begin(), this->buffer_resources.end()};
			break;
			case ResourceType::Texture:
				return {this->texture_resources.begin(), this->texture_resources.end()};
			break;
			default:
				tz_error("Unknown ResourceType when retrieving a resource span");
				return {};
			break;
		}
	}

	void RendererBuilderBase::set_culling_strategy(RendererCullingStrategy culling_strategy)
	{
		this->culling_strategy = culling_strategy;
	}

	RendererCullingStrategy RendererBuilderBase::get_culling_strategy() const
	{
		return this->culling_strategy;
	}

	void RendererBuilderBase::set_shader(const IShader& shader)
	{
		this->shader = &shader;
	}

	const IShader& RendererBuilderBase::get_shader() const
	{
		tz_assert(this->shader != nullptr, "No shader yet");
		return *this->shader;
	}

	RendererBase::RendererBase(const IRendererBuilder& builder):
	inputs(RendererBase::copy_inputs(builder)),
	buffer_resources(RendererBase::copy_resources(ResourceType::Buffer, builder)),
	texture_resources(RendererBase::copy_resources(ResourceType::Texture, builder)),
	clear_colour{0.0f, 0.0f, 0.0f, 0.0f}
	{

	}

	void RendererBase::set_clear_colour(tz::Vec4 clear_colour)
	{
		this->clear_colour = clear_colour;
	}

	tz::Vec4 RendererBase::get_clear_colour() const
	{
		return this->clear_colour;
	}

	std::size_t RendererBase::input_count() const
	{
		return this->inputs.size();
	}
	
	std::size_t RendererBase::input_count_of(RendererInputDataAccess access) const
	{
		auto matches = [access](const std::unique_ptr<IRendererInput>& input_ptr)
		{
			tz_assert(input_ptr != nullptr, "Nullptr input detected. Having no inputs is okay, but having nullptrs for inputs is not.");
			return input_ptr->data_access() == access;
		};

		return std::count_if(this->inputs.begin(), this->inputs.end(), matches);
	}

	IRendererInput* RendererBase::get_input(RendererInputHandle handle)
	{
		auto handval = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
		return this->inputs[handval].get();
	}

	std::size_t RendererBase::resource_count() const
	{
		return this->buffer_resources.size() + this->texture_resources.size();
	}

	std::size_t RendererBase::resource_count_of(ResourceType type) const
	{
		switch(type)
		{
			case ResourceType::Buffer:
				return this->buffer_resources.size();
			break;
			case ResourceType::Texture:
				return this->texture_resources.size();
			break;
			default:
				tz_error("Unknown ResourceType");
				return 0;
			break;
		}
	}

	IResource* RendererBase::get_resource(ResourceHandle handle)
	{
		// We can't simply retrieve this->components.get_component(handle)->get_resource() // because that gives us a const resource. we must retrieve the original.
		auto handval = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
		const std::size_t bufc = this->resource_count_of(ResourceType::Buffer);
		const std::size_t texc = this->resource_count_of(ResourceType::Texture);
		if(handval < bufc)
		{
			// It's definitely a buffer
			return this->buffer_resources[handval].get();
		}
		else
		{
			// Either invalid or a texture.
			if(handval < bufc + texc)
			{
				// It's a texture
				return this->texture_resources[handval - bufc].get();
			}
			else
			{
				// It's invalid.
				tz_error("Invalid ResourceHandle");
				return nullptr;
			}
		}
	}

	/*static*/ std::vector<std::unique_ptr<IRendererInput>> RendererBase::copy_inputs(const IRendererBuilder& builder)
	{
		std::vector<std::unique_ptr<IRendererInput>> input_duplicates;
		for(std::size_t i = 0; i < builder.input_count(); i++)
		{
			RendererInputHandle handle{static_cast<tz::HandleValue>(i)};
			const IRendererInput* cur_input = builder.get_input(handle);
			tz_assert(cur_input != nullptr, "Builder had a null input. It is valid for a builder to have no inputs, but a null input is wrong.");
			input_duplicates.push_back(cur_input->unique_clone());
		}
		return input_duplicates;
	}

	/*static*/ std::vector<std::unique_ptr<IResource>> RendererBase::copy_resources(ResourceType type, const IRendererBuilder& builder)
	{
		std::vector<std::unique_ptr<IResource>> resource_duplicates;
		for(const IResource* const resource : builder.get_resources(type))
		{
			tz_assert(resource != nullptr, "Builder had a nullptr resource. This is not allowed");
			resource_duplicates.push_back(resource->unique_clone());
		}
		return resource_duplicates;
	}
}
