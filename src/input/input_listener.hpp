//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_INPUT_LISTENER_HPP
#define TOPAZ_INPUT_LISTENER_HPP
#include "input/input_event.hpp"

namespace tz::input
{
	/**
     * \addtogroup tz_input Topaz Input Library (tz::input)
     * A collection of platform-agnostic input functions and classes.
     * @{
     */

	struct KeyListener
	{
		virtual void on_key_press(KeyPressEvent kpe) = 0;
		virtual ~KeyListener() = default;
	};
	
	struct TypeListener
	{
		virtual void on_key_type(CharPressEvent cpe) = 0;
		virtual ~TypeListener() = default;
	};

	struct MouseListener
	{
		virtual void on_mouse_update(MouseUpdateEvent mue) = 0;
		virtual void on_mouse_click(MouseClickEvent mce) = 0;
		virtual ~MouseListener() = default;
	};
	
	template<typename Callback>
	struct CustomKeyListener : public KeyListener
	{
		explicit CustomKeyListener(Callback callback);
		virtual void on_key_press(KeyPressEvent kpe) override;
	private:
		Callback callback;
	};
	
	template<typename Callback>
	struct CustomTypeListener : public TypeListener
	{
		explicit CustomTypeListener(Callback callback);
		virtual void on_key_type(CharPressEvent cpe) override;
	private:
		Callback callback;
	};

	template<typename CallbackUpdate, typename CallbackClick>
	struct CustomMouseListener : public MouseListener
	{
		explicit CustomMouseListener(CallbackUpdate update, CallbackClick click);
		virtual void on_mouse_update(MouseUpdateEvent mue) override;
		virtual void on_mouse_click(MouseClickEvent mce) override;
	private:
		CallbackUpdate update;
		CallbackClick click;
	};

	/**
	 * @}
	 */
}

#include "input/input_listener.inl"
#endif //TOPAZ_INPUT_LISTENER_HPP
