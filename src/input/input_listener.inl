#include <algorithm>

namespace tz::input
{
	template<typename Callback>
	CustomKeyListener<Callback>::CustomKeyListener(Callback callback): callback(callback), pressed_keys(){}
	
	template<typename Callback>
	void CustomKeyListener<Callback>::on_key_press(KeyPressEvent kpe)
	{
		this->callback(kpe);
		if(kpe.action == KeyPressEvent::Action::Pressed)
		{
			this->pressed_keys.insert(kpe.key);
		}
		else if(kpe.action == KeyPressEvent::Action::Released)
		{
			this->pressed_keys.erase(kpe.key);
		}
	}

	template<typename Callback>
	bool CustomKeyListener<Callback>::is_key_down(KeyPressEvent::Key key) const
	{
		return this->pressed_keys.find(key) != this->pressed_keys.end();
	}

	template<typename Callback>
	bool CustomKeyListener<Callback>::is_key_up(KeyPressEvent::Key key) const
	{
		return this->pressed_keys.find(key) == this->pressed_keys.end();
	}
	
	template<typename Callback>
	CustomTypeListener<Callback>::CustomTypeListener(Callback callback): callback(callback){}
	
	template<typename Callback>
	void CustomTypeListener<Callback>::on_key_type(CharPressEvent cpe)
	{
		this->callback(cpe);
	}

	template<typename CallbackUpdate, typename CallbackClick>
	CustomMouseListener<CallbackUpdate, CallbackClick>::CustomMouseListener(CallbackUpdate update, CallbackClick click): update(update), click(click){}
	
	template<typename CallbackUpdate, typename CallbackClick>
	void CustomMouseListener<CallbackUpdate, CallbackClick>::on_mouse_update(MouseUpdateEvent mue)
	{
		this->update(mue);
	}

	template<typename CallbackUpdate, typename CallbackClick>
	void CustomMouseListener<CallbackUpdate, CallbackClick>::on_mouse_click(MouseClickEvent mce)
	{
		this->click(mce);
	}
}