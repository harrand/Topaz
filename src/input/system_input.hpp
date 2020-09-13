//
// Created by Harrand on 25/12/2019.
//

#ifndef TOPAZ_SYSTEM_INPUT_HPP
#define TOPAZ_SYSTEM_INPUT_HPP

namespace tz::input
{
	/**
	 * \addtogroup tz_input Topaz Input Library (tz::input)
	 * A collection of platform-agnostic input functions and classes.
	 * @{
	 */

	/**
	 * TODO: Document
	 */
	const char* get_clipboard_data();

	/**
	 * TODO: Document
	 */
	void set_clipboard_data(const char* data);

	/**
	 * @}
	 */
}

#endif // TOPAZ_SYSTEM_INPUT_HPP