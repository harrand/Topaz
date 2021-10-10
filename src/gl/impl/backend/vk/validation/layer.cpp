#include "gl/impl/backend/vk/validation/layer.hpp"
#if TZ_VULKAN
#include <cstdint>
#include "core/assert.hpp"
#include "vulkan/vulkan.h"

namespace tz::gl::vk
{
	ValidationLayerList get_all_available_layers()
	{
		std::uint32_t layer_count;
		auto res = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		tz_assert(res == VK_SUCCESS, "no", 5);
		std::vector<VkLayerProperties> available_layers(layer_count);
		res = vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
		tz_assert(res == VK_SUCCESS, "no", 5);

		ValidationLayerList avail_layers;
		for(const VkLayerProperties& avail_layer_prop : available_layers)
		{
			avail_layers.add(avail_layer_prop.layerName);
		}
		return avail_layers;
	}

	ValidationLayerList get_default_validation_layers()
	{
		#if TZ_DEBUG
		if constexpr(!validation::layers_enabled)
		{
			return {};
		}
		constexpr const char* default_layer_names[] = {"VK_LAYER_KHRONOS_validation"};
		constexpr std::size_t num_default_layers = sizeof(default_layer_names) / sizeof(const char*);
		ValidationLayerList avail_layers = get_all_available_layers();
		ValidationLayerList default_layers;
		for(std::size_t i = 0; i < num_default_layers; i++)
		{
			VulkanValidationLayer layer = default_layer_names[i];
			tz_assert(avail_layers.contains(layer), "tz::gl::vk::get_default_validation_layers(): Default layer named \"%s\" is not within the list of available validation layers!", layer.c_str());
			default_layers.add(layer);
		}
		return default_layers;
		#else
		return {};
		#endif
	}
}
#endif