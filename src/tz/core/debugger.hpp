#ifndef TOPAZ_CORE_DEBUGGER_HPP
#define TOPAZ_CORE_DEBUGGER_HPP

namespace tz
{
	/**
	 * @ingroup tz_core_utility
	 * Cause the debugger to break. If no debugger is present, the behaviour is unspecified. You should assume it causes program termination.
	 */
	void debug_break();
}

#endif // TOPAZ_CORE_DEBUGGER_HPP
