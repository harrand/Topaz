#ifndef TOPAZ_GL_VK_VERTEX_BINDING_DESCRIPTION_HPP
#define TOPAZ_GL_VK_VERTEX_BINDING_DESCRIPTION_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"
#include "core/assert.hpp"
#include <cstdint>

namespace tz::gl::vk
{
	enum class VertexInputRate
	{
		PerVertexBasis,
		PerInstanceBasis
	};

	class VertexBindingDescription
	{
	public:
		constexpr VertexBindingDescription(std::uint32_t binding, std::size_t stride, VertexInputRate rate):
		desc()
		{
			this->desc.binding = binding;
			this->desc.stride = static_cast<std::uint32_t>(stride);
			switch(rate)
			{
				case VertexInputRate::PerVertexBasis:
					this->desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				break;
				case VertexInputRate::PerInstanceBasis:
					this->desc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
				break;
				default:
					tz_error("Unrecognised VertexInputRate");
				break;
			}
		}

		VkVertexInputBindingDescription native() const;
	private:
		VkVertexInputBindingDescription desc;
	};

	using VertexBindingDescriptions = std::initializer_list<VertexBindingDescription>;
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_VERTEX_BINDING_DESCRIPTION_HPP