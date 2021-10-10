#include "core/assert.hpp"
namespace tz::gl
{
	template<typename ComponentType, typename... Ts>
	ResourceHandle ComponentManager::emplace_component(Ts&&... args)
	{
		static_assert(std::is_base_of_v<IComponent, ComponentType>, "ComponentType must inherit from IComponent");
		auto component_ptr = std::make_unique<ComponentType>(std::forward<Ts>(args)...);
		switch(component_ptr->get_resource()->get_type())
		{
			case ResourceType::Buffer:
				this->owned_buffer_components.push_back(std::move(component_ptr));
				return this->add_component(this->owned_buffer_components.back().get());
			break;
			case ResourceType::Texture:
				this->owned_texture_components.push_back(std::move(component_ptr));
				return this->add_component(this->owned_texture_components.back().get());
			break;
			default:
				tz_error("Unrecognised ResourceType");
				return tz::HandleValue{0};
			break;
		}
	}
}