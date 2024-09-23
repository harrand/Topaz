#ifndef TZ_GPU_DEVICE_HPP
#define TZ_GPU_DEVICE_HPP
#include "tz/core/handle.hpp"
#include <string>
#include <span>
#include <cstddef>

namespace tz::gpu
{
	/**
	 * @ingroup tz_gpu
	 * @defgroup tz_gpu_device Devices and Hardware
	 * @brief Documentation specialised for iterating over rendering hardware and creating devices to interface with them.
	 **/

	/**
	 * @ingroup tz_gpu_device
	 * @brief Error codes for the @ref tz_gpu
	 **/
	enum class error_code
	{
		/// Correct behaviour has occurred. No need to do any sanity checking.
		success,
		/// Nothing erroneous happened, but the process did not complete fully or otherwise provide an ideal result.
		partial_success,
		/// An error has occurred because some previously-required setup has not been complete. The most common cause of this is not initialising the engine via @ref tz::initialise
		precondition_failure,
		/// An error has occurred, but it's not clear why.
		unknown_error,
		/// An error has occurred due to lack of CPU memory.
		oom,
		/// An error has occurred due to lack of GPU memory.
		voom,
		_count
	};

	/**
	 * @ingroup tz_gpu_device
	 * @brief Describes a specific type of rendering hardware.
	 **/
	enum class hardware_type
	{
		/// A discrete GPU, the most ideal hardware for graphics/compute. Typically seen on desktop PCs.
		gpu,
		/// An integrated GPU, typically seen on laptops.
		integrated_gpu,
		/// A CPU. One should expect subpar performance, but there may be reasons to use a CPU.
		cpu,
		/// Your graphics driver failed to identify the hardware, or Topaz does not support whatever it is. You should not attempt to create a device using this hardware.
		unknown
	};

	/**
	 * @ingroup tz_gpu_device
	 * @brief Describes what kind of GPU operations a rendering hardware can carry out when used.
	 **/
	enum class hardware_capabilities
	{
		/// Both graphics and compute. These are always preferred.
		graphics_compute,
		/// Graphics only. Attempting to carry out compute work on this hardware will fail.
		graphics_only,
		/// Compute only. Attempting to carry out graphics work (rasterisation) on this hardware will fail.
		compute_only,
		/// The hardware is useless for Topaz. You should not attempt to create a device using this hardware.
		neither
	};

	/**
	 * @ingroup tz_gpu_device
	 * @brief Describes to what extent the rendering hardware supports all the features that Topaz requires.
	 **/
	enum class hardware_feature_coverage
	{
		/// The hardware supports everything Topaz needs to do to support all @ref tz_gpu features.
		ideal,
		/// The hardware does not support all features, and as such could crash.
		insufficient,
		/// The hardware does not come close to supporting all required features, and as such should not be attempted to be used.
		poor,
		_count
	};

	using hardware_handle = tz::handle<hardware_type>;


	/**
	 * @ingroup tz_gpu_device
	 * @brief Contains some basic information about a particular piece of hardware on the machine that the driver thinks could do GPU work.
	 *
	 * If you're looking to request information about the hardware available on your machine, look through @ref iterate_hardware.
	 **/
	struct hardware
	{
		/// User-facing name of the hardware. In most cases this should be the name of your graphics card. Note that some drivers may truncate this to an implementation-defined maximum size.
		std::string name;
		/// Estimated size of the biggest heap, in MiB. If the hardware is not a discrete GPU, this value could be inaccurate.
		std::uint64_t vram_size_mib;
		/// Describes which type of hardware this is (e.g a discrete GPU, or a CPU).
		hardware_type type;
		/// Describes whether this hardware is suited to Topaz rendering based upon the GPU features it supports.
		hardware_feature_coverage features;
		/// Describes which sort of GPU operations this hardware is capable of doing.
		hardware_capabilities caps;

		struct internals_t
		{
			hardware_handle i0;
			std::uint32_t i1;
		} internals;
	};

	/**
	 * @ingroup tz_gpu_device
	 * @brief Opaque handle corresponding to a previously-created device.
	 * 
	 * A device is a fully-realised interface through some rendering hardware running on the machine. To create a device, see @ref create_device.
	 **/
	using device_handle = tz::handle<const hardware>;
	/**
	 * @ingroup tz_gpu_device
	 * @brief Retrieve information about all detected rendering hardware currently available on the machine.
	 * @param devices A region of memory through which hardware information shall be written to. As many hardware components as possible will be written, until all hardware has been listed or the region does not have enough available space.
	 * @param hardware_count A pointer to a value which (if not-null) will be updated with the total number of rendering hardware components available on the machine.
	 * @return - @ref error_code::partial_success If the span of devices provides is not large enough to store all devices on the machine.
	 * @return - @ref error_code::precondition_failure If Topaz has not yet been initialised via @ref tz::initialise.
	 * @return - @ref error_code::unknown_error If some other error occurs.
	 *
	 * If you aren't interested in choosing a device yourself, and are happy to let Topaz figure out the best for you, use @ref find_best_hardware instead of this.
	 *
	 * You should use this function if you:
	 * - Want to examine all hardware available on the machine.
	 * - Know what you're doing and have a very specific set of requirements for your application, so much so that you cannot trust Topaz to give you the ideal result via @ref find_best_hardware.
	 **/
	error_code iterate_hardware(std::span<hardware> devices, std::size_t* hardware_count = nullptr);
	/**
	 * @ingroup tz_gpu_device
	 * @brief Retrieve the "best" hardware device on your machine.
	 * 
	 * This function prefers desktop GPUs that handle compute/graphics/transfer on the same queue.
	 *
	 * If you want more control over which hardware is used, or you don't trust Topaz's opinion, consider calling @ref iterate_hardware and choosing one yourself.
	 *
	 * Unlike @ref iterate_hardware, any error that the driver reports is considered fatal and will emit a @ref tz_error.
	 **/
	hardware find_best_hardware();
	/**
	 * @ingroup tz_gpu_device
	 * @brief Select a piece of hardware to use for future graphical operations.
	 * @param hw A hardware component of your choice that you wish to use to perform some GPU work.
	 *
	 * You must select a piece of hardware using this API call before attempting to submit any GPU work. There is no default device selection.

	 * @note All previous tz::gpu state will be invalidated if you use a new piece of hardware. It is recommended to select your hardware once after @ref tz::initialise, use it with this function and never attempt to use other hardware.
	 **/
	error_code use_hardware(hardware hw);
	/**
	 * @ingroup tz_gpu_device
	 * @brief Destroy an existing logical device.
	 * @param device Logical device from a previous call to @ref create_device.
	 *
	 * You should call this to cleanup resources after you have completed whatever rendering work you needed to do with the hardware.
	 * @note Creating and destroying devices should be considered *extremely* slow. For that reason, only create a device when you are sure you need to do some GPU work, and only destroy it once you are certain you have no further need of it.
	 **/
	void destroy_device(device_handle device);
}

#endif // TZ_GPU_DEVICE_HPP