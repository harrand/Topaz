#ifndef TOPAZ_GPU_SETTINGS_HPP
#define TOPAZ_GPU_SETTINGS_HPP

namespace tz::gpu
{
	/**
	 * @ingroup tz_gpu
	 * @defgroup tz_gpu_settings Settings
	 * @brief Global settings that affect all passes and graphs. The kind of stuff you'd expect to see in a game's video options.
	 */

	 /**
	  * @ingroup tz_gpu_settings
	  * @brief Query as to whether vsync is enabled.
	  *
	  * By default, vsync is disabled.
	  */
	bool settings_get_vsync();
	/**
	 * @ingroup tz_gpu_settings
	 * @brief Enable/disable vsync.
	 *
	 * By default, vsync is disabled. You should assume that this function will take a significantly long time.
	 */
	void settings_set_vsync(bool enabled);
}

#endif // TOPAZ_GPU_SETTINGS_HPP