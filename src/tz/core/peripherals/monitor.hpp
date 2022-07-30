#ifndef TOPAZ_CORE_PERIPHERALS_MONITOR_HPP
#define TOPAZ_CORE_PERIPHERALS_MONITOR_HPP
#include "tz/core/vector.hpp"
#include "tz/core/containers/basic_list.hpp"

namespace tz
{
	namespace detail::peripherals::monitor
	{
		void initialise();
		void terminate();
	}

	/**
	 * @ingroup tz_core_peripherals
	 * @defgroup tz_core_peripherals_monitor Monitors
	 * Documentation for monitor properties.
	 */

	/**
	 * @ingroup tz_core_peripherals_monitor
	 * Describes properties about a specific monitor.
	 */
	struct MonitorInfo
	{
		/// Index of the monitor. The primary monitor is guaranteed to be of index '0'. Each monitor has a unique index.
		unsigned int monitor_index;
		/**
		 * Contains the real-life dimensions of the monitor (or a close estimation), in metric millimetres.
		 */
		tz::Vec2ui physical_dimensions_millimetres;
		/**
		 * Contains the dimensions of the monitor screen, in screen coordinates.
		 * @note Screen coordinates do not necessarily map 1:1 to pixels. See @ref content_scale for a silver-bullet.
		 */
		tz::Vec2ui screen_dimensions;
		/**
		 * Contains the ratio between current DPI and the platform's default DPI. You can component-wise-multiply this with a pair of screen coordinates to map them to pixels. Oftentimes this will be {1.0f, 1.0f} but this is not guaranteed, for example on a Mac with a retina display.
		 */
		tz::Vec2 content_scale;
		/**
		 * Represents a human-readable name for the monitor. This is not guaranteed to be unique, nor a particularly descriptive name.
		 */
		const char* name;
	};

	/**
	 * @ingroup tz_core_peripherals_monitor
	 * Retrieve the default/primary monitor. This is usually the one with global UI elements such as the task or menu bar.
	 * @pre At least one monitor must be connected, otherwise the behaviour is undefined. To retrieve monitor information safely if you cannot guarantee this, retrieve the first result of @ref get_monitors() if its not empty.
	 */
	MonitorInfo get_default_monitor();
	/**
	 * @ingroup tz_core_peripherals_monitor
	 * Retrieve a list of all monitors, sorted by their ids. If no monitors are detected, returns an empty list.
	 * @note The default monitor (see @ref get_default_monitor()) is guaranteed to be the first element in this list.
	 */
	tz::BasicList<MonitorInfo> get_monitors();
}

#endif // TOPAZ_CORE_PERIPHERALS_MONITOR_HPP
