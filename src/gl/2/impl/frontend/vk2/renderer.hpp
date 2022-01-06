#ifndef TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP
#if TZ_VULKAN
#include "gl/2/api/renderer.hpp"
#include "gl/2/api/component.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl2
{
	using namespace tz::gl;
	template<class Asset>
	class AssetStorage
	{
	public:
		using AssetHandle = tz::Handle<Asset>;
		AssetStorage(std::span<const Asset* const> assets):
		asset_storage()
		{
			for(const Asset* asset : assets)
			{
				this->asset_storage.push_back(asset->unique_clone());
			}
		}
		unsigned int count() const
		{
			return this->asset_storage.size();
		}
		
		const Asset* get(AssetHandle handle) const
		{
			std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
			return this->asset_storage[handle_val].get();
		}

		Asset* get(AssetHandle handle)
		{
			std::size_t handle_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
			return this->asset_storage[handle_val].get();
		}
	private:
		std::vector<std::unique_ptr<Asset>> asset_storage;
	};

	using InputStorage = AssetStorage<IInput>;

	class ResourceStorage : public AssetStorage<IResource>
	{
	public:
		ResourceStorage(std::span<const IResource* const> resources, const vk2::LogicalDevice& ldev);
		const IComponent* get_component(ResourceHandle handle) const;
		IComponent* get_component(ResourceHandle handle);
	private:
		std::vector<std::unique_ptr<IComponent>> components;
	};

	class RendererInfoVulkan
	{
	public:
		RendererInfoVulkan() = default;
		// Satisfies RendererInfoType.
		unsigned int input_count() const;
		const IInput* get_input(InputHandle handle);
		std::span<const IInput* const> get_inputs() const;
		unsigned int resource_count() const;
		const IResource* get_resource(ResourceHandle handle);
		std::span<const IResource* const> get_resources() const;
		InputHandle add_input(IInput& input);
		ResourceHandle add_resource(IResource& resource);
	private:
		std::vector<IInput*> inputs = {};
		std::vector<IResource*> resources = {};
	};
	static_assert(RendererInfoType<RendererInfoVulkan>);

	struct RendererDeviceInfoVulkan
	{
		const vk2::LogicalDevice* vk_device;
	};

	class RendererVulkan
	{
	public:
		RendererVulkan(const RendererInfoVulkan& info, const RendererDeviceInfoVulkan& device_info);
		// Satisfies RendererType
		unsigned int input_count() const;
		const IInput* get_input(InputHandle handle) const;
		IInput* get_input(InputHandle handle);
		unsigned int resource_count() const;
		const IResource* get_resource(ResourceHandle handle) const;
		IResource* get_resource(ResourceHandle handle);
		const IComponent* get_component(ResourceHandle handle) const;
		IComponent* get_component(ResourceHandle handle);

		void render(){}
	private:
		InputStorage inputs;
		ResourceStorage resources;
	};

	static_assert(RendererType<RendererVulkan>);
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL2_IMPL_FRONTEND_VK2_RENDERER_HPP
