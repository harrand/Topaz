#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
#if TZ_VULKAN
#include "hdk/data/vector.hpp"
#include "tz/core/containers/basic_list.hpp"
#include "tz/core/containers/enum_field.hpp"
#include "tz/gl/impl/vulkan/detail/tz_vulkan.hpp"
#include "tz/gl/impl/vulkan/detail/image_format.hpp"
#include "tz/gl/impl/vulkan/detail/features.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk
	 * Represents a PhysicalDevice manufacturer.
	 */
	enum class PhysicalDeviceVendor
	{
		Nvidia,
		AMD,
		Intel,
		Other
	};

	enum class PhysicalDeviceType
	{
		IntegratedGPU,
		DiscreteGPU,
		VirtualGPU,
		CPU,
		Unknown
	};

	struct InternalDeviceInfo
	{
		VkPhysicalDeviceMemoryProperties memory;
		VkPhysicalDeviceLimits limits;
	};

	struct PhysicalDeviceInfo
	{
		PhysicalDeviceVendor vendor;
		PhysicalDeviceType type;
		std::string name;
		InternalDeviceInfo internal;
	};

	struct PhysicalDeviceSurfaceCapabilityInfo
	{
		PhysicalDeviceSurfaceCapabilityInfo(VkSurfaceCapabilitiesKHR vk_capabilities);
		struct SwapchainExtent
		{
			hdk::vector<std::uint32_t, 2> current_extent;
			hdk::vector<std::uint32_t, 2> min_image_extent;
			hdk::vector<std::uint32_t, 2> max_image_extent;
		};
		std::uint32_t min_image_count;
		std::uint32_t max_image_count;
		VkSurfaceTransformFlagBitsKHR current_transform;
		std::optional<SwapchainExtent> maybe_surface_dimensions;
	};

	namespace detail
	{
		/*
		 * Convert a vkPhysicalDeviceFeatures to a Topaz Vulkan feature.
		 * Note: If none of the enabled features are Topaz Vulkan features, the resultant field will be empty.
		 */
		DeviceFeatureField to_feature_field(DeviceFeatureInfo vulkan_features);
		/*
		 * Convert a Topaz Vulkan feature into its vulkan-api-friendly variant.
		 */
		DeviceFeatureInfo from_feature_field(const DeviceFeatureField& feature_field);

		PhysicalDeviceVendor to_tz_vendor(VkDriverId driver_id);
		PhysicalDeviceType to_tz_type(VkPhysicalDeviceType type);
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
		/**
		 * @brief PhysicalDevices do not necessarily support all available DeviceFeatures.
		 * @return An EnumField containing all the features supported by this Physical device.
		 */
		DeviceFeatureField get_supported_features() const;
		/**
		 * @brief PhysicalDevices support various extensions, but not necessarily all of them.
		 * @return An EnumField containing all the extensions supported by this PhysicalDevice.
		 */
		DeviceExtensionList get_supported_extensions() const;
		/**
		 * Retrieve the vendor. Only a few vendors are supported, so check @ref PhysicalDeviceVendor for information.
		 * If you're using very new hardware, or various custom drivers such as MoltenVK, this may very well not return the expected value. This should mainly be used for vendor-specific optimisations and shouldn't be exposed to the end-user.
		 */
		PhysicalDeviceInfo get_info() const;
		/**
		 * Retrieve a list of all image_formats that could represent the given window surface.
		 * @return List of acceptable image_formats for a window surface.
		 */
		tz::BasicList<image_format> get_supported_surface_formats() const;
		/**
		 * Retrieve a list of all SurfacePresentModes that could be used to present images to the @ref WindowSurface attached to the @ref VulkanInstance owning this device.
		 * This is guaranteed to contain every element within @ref present_traits::get_mandatory_present_modes().
		 * @return List of acceptable SurfacePresentModes for a window surface.
		 */
		tz::BasicList<SurfacePresentMode> get_supported_surface_present_modes() const;
		PhysicalDeviceSurfaceCapabilityInfo get_surface_capabilities() const;
		/**
		 * Query as to whether the given image_format can be used as a framebuffer colour attachment and as an input attachment format.
		 * This is guaranteed to return true for any @ref image_format within @ref format_traits::get_mandatory_colour_attachment_formats().
		 * @return true if `colour_format` can be a colour attachment, otherwise false.
		 */
		bool supports_image_colour_format(image_format colour_format) const;
		/**
		 * Query as to whether an ImageView using this format can be sampled from within a shader. 
		 * @return true if `sampled_format` can be sampled in a shader, otherwise false.
		 */
		bool supports_image_sampled_format(image_format sampled_format) const;
		/**
		 * Query as to whether the given image_format can be used as a framebuffer depth/stencil attachment and as an input attachment format.
		 * @return true if `depth_format` can be a depth/stencil attachment, otherwise false.
		 */
		bool supports_image_depth_format(image_format depth_format) const;
		/**
		 * Retrieve the @ref VulkanInstance to which this physical device belongs.
		 */
		const VulkanInstance& get_instance() const;

		using NativeType = VkPhysicalDevice;
		NativeType native() const;
		static PhysicalDevice null(); 
		bool is_null() const;
		bool operator==(const PhysicalDevice& rhs) const;
	private:
		struct DeviceProps
		{
			VkPhysicalDeviceProperties2 props = {};
			VkPhysicalDeviceDriverProperties driver_props = {};
		};
		PhysicalDevice();
		DeviceProps get_internal_device_props() const;
		bool supports_image_format(image_format format, VkFormatFeatureFlagBits feature_type) const;

		VkPhysicalDevice dev;
		const VulkanInstance* instance;
	};

	using PhysicalDeviceList = tz::BasicList<PhysicalDevice>;

	/*
	 * @ingroup tz_gl_vk
	 */
	PhysicalDeviceList get_all_devices();
	/**
	 * @ingroup tz_gl_vk
	 * @brief Retrieve a list of all physical devices available on the machine for the given @ref VulkanInstance.
	 * If no VulkanInstance is provided, the global instance @ref vk2::get() will be used.
	 * @return BasicList of all PhysicalDevices. These have not been filtered in any way.
	 */
	PhysicalDeviceList get_all_devices(const VulkanInstance& instance);
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
