#if TZ_VULKAN
#include "gl/2/impl/frontend/vk2/renderer.hpp"
#include "gl/2/impl/frontend/vk2/component.hpp"

namespace tz::gl2
{
	using namespace tz::gl;

	ResourceStorage::ResourceStorage(std::span<const IResource* const> resources, const vk2::LogicalDevice& ldev):
	AssetStorage<IResource>(resources)
	{
		for(std::size_t i = 0; i < this->count(); i++)
		{
			IResource* res = this->get(static_cast<tz::HandleValue>(i));
			switch(res->get_type())
			{
				case ResourceType::Buffer:
					this->components.push_back(std::make_unique<BufferComponentVulkan>(*res, ldev));
				break;
				case ResourceType::Image:
					this->components.push_back(std::make_unique<ImageComponentVulkan>(*res, ldev));
				break;
				default:
					tz_error("Unrecognised ResourceType. Please submit a bug report.");
				break;
			}
		}
	}

	const IComponent* ResourceStorage::get_component(ResourceHandle handle) const
	{
		return this->components[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))].get();
	}

	IComponent* ResourceStorage::get_component(ResourceHandle handle)
	{
		return this->components[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))].get();
	}

//--------------------------------------------------------------------------------------------------
	unsigned int RendererInfoVulkan::input_count() const
	{
		return this->inputs.size();
	}

	const IInput* RendererInfoVulkan::get_input(InputHandle handle)
	{
		return this->inputs[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))];
	}

	std::span<const IInput* const> RendererInfoVulkan::get_inputs() const
	{
		return this->inputs;
	}

	unsigned int RendererInfoVulkan::resource_count() const
	{
		return this->resources.size();
	}

	const IResource* RendererInfoVulkan::get_resource(ResourceHandle handle)
	{
		return this->resources[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))];
	}

	std::span<const IResource* const> RendererInfoVulkan::get_resources() const
	{
		return this->resources;
	}

	InputHandle RendererInfoVulkan::add_input(IInput& input)
	{
		this->inputs.push_back(&input);
		return static_cast<tz::HandleValue>(this->inputs.size() - 1);
	}

	ResourceHandle RendererInfoVulkan::add_resource(IResource& resource)
	{
		this->resources.push_back(&resource);
		return static_cast<tz::HandleValue>(this->resources.size() - 1);
	}

//--------------------------------------------------------------------------------------------------

	RendererVulkan::RendererVulkan(const RendererInfoVulkan& info, const RendererDeviceInfoVulkan& device_info):
	inputs(info.get_inputs()),
	resources(info.get_resources(), *device_info.vk_device)
	{

	}

	unsigned int RendererVulkan::input_count() const
	{
		return this->inputs.count();
	}

	const IInput* RendererVulkan::get_input(InputHandle handle) const
	{
		return this->inputs.get(handle);
	}

	IInput* RendererVulkan::get_input(InputHandle handle)
	{
		return this->inputs.get(handle);
	}

	unsigned int RendererVulkan::resource_count() const
	{
		return this->resources.count();
	}

	const IResource* RendererVulkan::get_resource(ResourceHandle handle) const
	{
		return this->resources.get(handle);
	}

	IResource* RendererVulkan::get_resource(ResourceHandle handle)
	{
		return this->resources.get(handle);
	}

	const IComponent* RendererVulkan::get_component(ResourceHandle handle) const
	{
		return this->resources.get_component(handle);
	}

	IComponent* RendererVulkan::get_component(ResourceHandle handle)
	{
		return this->resources.get_component(handle);
	}
}

#endif // TZ_VULKAN
