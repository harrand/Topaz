#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_LOGICAL_DEVICE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_LOGICAL_DEVICE_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "gl/impl/backend/vk2/hardware/queue.hpp"

namespace tz::gl::vk2
{
	/**
	 * @brief PhysicalDevices contain families of queues. Each family can fulfill a variety of different purposes, such as graphics or compute. This enum lists all such usages.
	 * @note Queue Families may have more than one type, and due to this are associated with a @ref QueueFamilyTypeField.
	 */
	enum class QueueFamilyType
	{
		/// - Indicates that queues within this family are suitable for graphics operations.
		Graphics,
		/// - Indicates that queues within this family are suitable for compute operations.
		Compute,
		/// - Indicates that queues within this family are suitable for transfer operations.
		Transfer
	};

	/**
	 * An @ref EnumField of QueueFamilyType. Can wholly describe the usage of one of a PhysicalDevice's queue families.
	 */
	using QueueFamilyTypeField = tz::EnumField<QueueFamilyType>;

	struct QueueFamilyInfo
	{
		std::uint32_t family_size;
		bool present_support;
		QueueFamilyTypeField types;
	};

	/**
	 * Specifies parameters for a newly created @ref LogicalDevice.
	 * @ingroup tz_gl_vk
	 */
	struct LogicalDeviceInfo
	{
		/// physical_device is the PhysicalDevice that the LogicalDevice shall be based off.
		PhysicalDevice physical_device = PhysicalDevice::null();
		/**
		 * `extensions` is a list of DeviceExtensions to be enabled.
		 * @pre For each element `e`, `physical_device.get_supported_extensions().contains(e)` must hold true.
		 */
		DeviceExtensionList extensions = {};
		/**
		 * `features` is a list of DeviceFeatures to be enabled.
		 * @pre For each element `e`, `physical_device.get_supported_features().contains(e)` must hold true.
		 */
		DeviceFeatureField features = {};
		/**
		 * `surface` is `nullptr` or a valid pointer to an existing @ref WindowSurface
		 * @pre If a surface is provided, the @ref VulkanInstance who spawned it must be equal to the VulkanInstance that spawned `physical_device`
		 */
		const WindowSurface* surface = nullptr;
	};

	struct QueueRequest
	{
		QueueFamilyTypeField field;
		bool present_support;
	};

	class QueueStorage
	{
	public:
		QueueStorage() = default;
		void init(std::span<const QueueFamilyInfo> queue_families, const LogicalDevice& device);
		const hardware::Queue* request_queue(QueueRequest request) const;
	private:
		struct QueueData
		{
			QueueData(hardware::QueueInfo queue_info, QueueFamilyInfo family_info);
			QueueData(const QueueData& copy) = delete;
			QueueData(QueueData&& move);
			~QueueData() = default;
			QueueData& operator=(const QueueData& rhs) = delete;
			QueueData& operator=(QueueData&& rhs);

			hardware::Queue queue;
			QueueFamilyInfo family;
		};
		using List = tz::BasicList<QueueData>;
		tz::BasicList<List> hardware_queue_families;	
	};
	
	/**
	 * @ingroup tz_gl_vk
	 * @brief Logical interface to an existing @ref PhysicalDevice
	 */
	class LogicalDevice
	{
	public:
		/**
		 * @brief Construct a LogicalDevice based on a PhysicalDevice, and some optional extensions/features to enable.
		 * @pre All elements of `enabled_extensions` are supported. That is, are contained within @ref PhysicalDevice::get_supported_extensions and @ref PhysicalDevice::get_supported_features. If an extension/feature is enabled which is not supported by `physical_device`, the behaviour is undefined.
		 */
		LogicalDevice(LogicalDeviceInfo device_info);
		LogicalDevice(const LogicalDevice& copy) = delete;
		LogicalDevice(LogicalDevice&& move);
		~LogicalDevice();
		LogicalDevice& operator=(const LogicalDevice& rhs) = delete;
		LogicalDevice& operator=(LogicalDevice&& rhs);
		/**
		 * @brief Retrieve the PhysicalDevice that this LogicalDevice derives from.
		 * @return PhysicalDevice provided to the original @ref LogicalDeviceInfo.
		 */
		const PhysicalDevice& get_hardware() const;
		/**
		 * @brief Retrieve a list of all enabled extensions.
		 */
		const DeviceExtensionList& get_extensions() const;
		/**
		 * @brief Retrieve a list of all enabled features.
		 */
		const DeviceFeatureField& get_features() const;

		const hardware::Queue* get_hardware_queue(QueueRequest request) const;

		using NativeType = VkDevice;
		NativeType native() const;
		/**
		 * Create a LogicalDevice which doesn't do anything.
		 * @note It is an error to use null LogicalDevices for most operations. Retrieving the native handle and querying for null-ness are the only valid operations on a null LogicalDevice.
		 */
		static LogicalDevice null();
		/**
		 * Query as to whether the LogicalDevice is null. Null LogicalDevices cannot perform operations or be used for GPU work.
		 * See @ref LogicalDevice::null for more information.
		 */
		bool is_null() const;
	private:
		LogicalDevice();

		VkDevice dev;
		PhysicalDevice physical_device;
		DeviceExtensionList enabled_extensions;
		DeviceFeatureField enabled_features;
		std::vector<QueueFamilyInfo> queue_families;
		QueueStorage queue_storage;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_LOGICAL_DEVICE_HPP
