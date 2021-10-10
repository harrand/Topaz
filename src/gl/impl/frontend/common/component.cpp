#include "gl/impl/frontend/common/component.hpp"

namespace tz::gl
{
	ResourceHandle ComponentManager::add_component(IComponent* component)
	{
		std::size_t resource_handle_value = this->total_components();
		switch(component->get_resource()->get_type())
		{
			case ResourceType::Buffer:
				this->buffer_components.push_back(component);
			break;
			case ResourceType::Texture:
				this->texture_components.push_back(component);
			break;
		}
		return tz::HandleValue{resource_handle_value};
	}

	const IComponent* ComponentManager::get_component(ResourceHandle handle) const
	{
		auto hand_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
		if(hand_val < this->buffer_components.size())
		{
			// It's a buffer.
			return this->buffer_components[hand_val];
		}
		else if(hand_val < this->buffer_components.size() + this->texture_components.size())
		{
			// It's a texture.
			return this->texture_components[hand_val - this->buffer_components.size()];
		}
		else
		{
			// It's broken.
			tz_error("Invalid ResourceHandle");
			return nullptr;
		}
	}

	IComponent* ComponentManager::get_component(ResourceHandle handle)
	{
		auto hand_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
		if(hand_val < this->buffer_components.size())
		{
			// It's a buffer.
			return this->buffer_components[hand_val];
		}
		else if(hand_val < this->buffer_components.size() + this->texture_components.size())
		{
			// It's a texture.
			return this->texture_components[hand_val - this->buffer_components.size()];
		}
		else
		{
			// It's broken.
			tz_error("Invalid ResourceHandle");
			return nullptr;
		}
	}

	std::span<const IComponent* const> ComponentManager::components(ResourceType type) const
	{
		switch(type)
		{
			case ResourceType::Buffer:
				return this->buffer_components;
			break;
			case ResourceType::Texture:
				return this->texture_components;
			break;
			default:
				tz_error("Invalid ResourceType");
				return {};
			break;
		}
	}

	std::span<IComponent* const> ComponentManager::components(ResourceType type)
	{
		switch(type)
		{
			case ResourceType::Buffer:
				return this->buffer_components;
			break;
			case ResourceType::Texture:
				return this->texture_components;
			break;
			default:
				tz_error("Invalid ResourceType");
				return {};
			break;
		}
	}

	std::size_t ComponentManager::component_count() const
	{
		return this->total_components();
	}

	std::size_t ComponentManager::component_count_of(ResourceType type) const
	{
		switch(type)
		{
			case ResourceType::Buffer:
				return this->buffer_components.size();
			break;
			case ResourceType::Texture:
				return this->texture_components.size();
			break;
			default:
				tz_error("Invalid ResourceType");
				return 0;
			break;
		}
	}

	std::size_t ComponentManager::total_components() const
	{
		return this->buffer_components.size() + this->texture_components.size();
	}

}