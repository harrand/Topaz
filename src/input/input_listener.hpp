//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_INPUT_LISTENER_HPP
#define TOPAZ_INPUT_LISTENER_HPP
#include "input/input_event.hpp"

namespace tz::input
{
	struct KeyListener
	{
		virtual void on_key_press(KeyPressEvent kpe) = 0;
	};
	
	struct TypeListener
	{
		virtual void on_key_type(CharPressEvent cpe) = 0;
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
}

#include "input/input_listener.inl"
#endif //TOPAZ_INPUT_LISTENER_HPP
