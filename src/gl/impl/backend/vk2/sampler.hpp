#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_SAMPLER_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_SAMPLER_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_image
	 * Represents a filter to a texture lookup.
	 */
	enum class LookupFilter
	{
		Nearest = VK_FILTER_NEAREST,
		Linear = VK_FILTER_LINEAR
	};

	/**
	 * @ingroup tz_gl_vk_image
	 * Specify mipmap mode for texture lookups.
	 */
	enum class MipLookupFilter
	{
		Nearest = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		Linear = VK_SAMPLER_MIPMAP_MODE_LINEAR
	};

	/**
	 * @ingroup tz_gl_vk_image
	 * Specifies behaviour with sampling with texture coordinates that lie outside of the image.
	 */
	enum class SamplerAddressMode
	{
		/// - The texture will repeat endlessly.
		Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		/// - The texture will repeat endlessly, but mirrored each time.
		MirroredRepeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		/// - The texture will have the same colour as the edge.
		ClampToEdge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		/// - The texture will have the same colour as the border.
		ClampToBorder = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	};

	/**
	 * @ingroup tz_gl_vk_image
	 * Specifies creation flags for a @ref Sampler.
	 */
	struct SamplerInfo
	{
		/// Owning LogicalDevice. This must not be null or nullptr.
		const LogicalDevice* device;

		/// Min filter.
		LookupFilter min_filter;
		/// Mag filter.
		LookupFilter mag_filter;
		MipLookupFilter mipmap_mode;
		/// Addressing mode for U coordinates outside of the region of the texture.
		SamplerAddressMode address_mode_u;
		/// Addressing mode for V coordinates outside of the region of the texture.
		SamplerAddressMode address_mode_v;
		/// Addressing mode for W coordinates outside of the region of the texture.
		SamplerAddressMode address_mode_w;
	};

	/**
	 * @ingroup tz_gl_vk_image
	 * Represents the state of an @ref Image sampler which is used to read image data and apply filtering and other transformations to a @ref Shader.
	 */
	class Sampler
	{
	public:
		Sampler(SamplerInfo info);
		Sampler(const Sampler& copy) = delete;
		Sampler(Sampler&& move);
		~Sampler();
		Sampler& operator=(const Sampler& rhs) = delete;
		Sampler& operator=(Sampler&& rhs);

		/**
		 * Retrieve the LogicalDevice used to create the sampler.
		 */
		const LogicalDevice& get_device() const;

		using NativeType = VkSampler;
		NativeType native() const;
	private:
		VkSampler sampler;
		SamplerInfo info;
	};

}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_SAMPLER_HPP
