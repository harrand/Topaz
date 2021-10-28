#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
#if TZ_VULKAN
#include "core/containers/basic_list.hpp"
#include "core/containers/enum_field.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"
#include "gl/impl/backend/vk2/image_format.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk
	 * @{
	 */

	/// Represents a Vulkan feature directly supported by Topaz.
	enum class PhysicalDeviceFeature
	{
		MultiDrawIndirect ///  - Allows mass-batching of draw-calls. Vastly improves performance for large scenes.
	};

	/// Represents a PhysicalDevice manufacturer.
	enum class PhysicalDeviceVendor
	{
		Nvidia,
		AMD,
		Intel,
		Other
	};

	using PhysicalDeviceFeatureField = tz::EnumField<PhysicalDeviceFeature>;

	/**
	 * @}
	 */

	namespace detail
	{
		/*
		 * Convert a vkPhysicalDeviceFeatures to a Topaz Vulkan feature.
		 * Note: If none of the enabled features are Topaz Vulkan features, the resultant field will be empty.
		 */
		PhysicalDeviceFeatureField to_feature_field(VkPhysicalDeviceFeatures features);
		/*
		 * Convert a Topaz Vulkan feature into its vulkan-api-friendly variant.
		 */
		VkPhysicalDeviceFeatures from_feature_field(const PhysicalDeviceFeatureField& feature_field);

		PhysicalDeviceVendor to_tz_vendor(VkDriverId driver_id);
	}

	/**
	 * @ingroup tz_gl_vk
	 * @brief Represents something resembling a graphics card that can perform general graphical operations. A PhysicalDevice may or may not support graphics, compute or transfer work.
	 */
	class PhysicalDevice
	{
	public:
		/**
		 * @brief You're not meant to construct these directly. See @ref get_all_devices()
		 */
		PhysicalDevice(VkPhysicalDevice native, const VulkanInstance& instance);
		PhysicalDevice();
		static PhysicalDevice null(); 
		/**
		 * @brief PhysicalDevices do not necessarily support all available PhysicalDeviceFeatures.
		 * @return An EnumField containing all the features supported by this Physical Device.
		 */
		PhysicalDeviceFeatureField get_supported_features() const;
		/**
		 * @brief PhysicalDevices support various extensions, but not necessarily all of them.
		 * @return An EnumField containing all the extensions supported by this PhysicalDevice.
		 */
		DeviceExtensionList get_supported_extensions() const;
		/**
		 * Retrieve the vendor. Only a few vendors are supported, so check @ref PhysicalDeviceVendor for information.
		 * If you're using very new hardware, or various custom drivers such as MoltenVK, this may very well not return the expected value. This should mainly be used for vendor-specific optimisations and shouldn't be exposed to the end-user.
		 */
		PhysicalDeviceVendor get_vendor() const;
		/**
		 * Retrieve a list of all ImageFormats that could represent the given window surface.
		 * @return List of acceptable ImageFormats for a window surface.
		 */
		tz::BasicList<ImageFormat> get_supported_surface_formats(const WindowSurface& surface) const;
		/**
		 * Query as to whether the given ImageFormat can be used as a framebuffer colour attachment and as an input attachment format.
		 * @return true if `colour_format` can be a colour attachment, otherwise false.
		 */
		bool supports_image_colour_format(ImageFormat colour_format) const;
		/**
		 * Query as to whether an ImageView using this format can be sampled from within a shader. 
		 * @return true if `sampled_format` can be sampled in a shader, otherwise false.
		 */
		bool supports_image_sampled_format(ImageFormat sampled_format) const;
		/**
		 * Query as to whether the given ImageFormat can be used as a framebuffer depth/stencil attachment and as an input attachment format.
		 * @return true if `depth_format` can be a depth/stencil attachment, otherwise false.
		 */
		bool supports_image_depth_format(ImageFormat depth_format) const;
		VkPhysicalDevice native() const;
	private:
		struct DeviceProps
		{
			VkPhysicalDeviceProperties2 props = {};
			VkPhysicalDeviceDriverProperties driver_props = {};
		};
		DeviceProps get_internal_device_props() const;
		bool supports_image_format(ImageFormat format, VkFormatFeatureFlagBits feature_type) const;

		VkPhysicalDevice dev;
		const VulkanInstance* instance;
	};

	using PhysicalDeviceList = tz::BasicList<PhysicalDevice>;

	/**
	 * @ingroup tz_gl_vk
	 * @brief Retrieve a list of all physical devices available on the machine
	 * @return BasicList of all PhysicalDevices. These have not been filtered in any way.
	 */
	PhysicalDeviceList get_all_devices();
	PhysicalDeviceList get_all_devices(const VulkanInstance& instance);
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
