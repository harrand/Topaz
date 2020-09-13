namespace tz::input
{
	template<typename Callback>
	CustomKeyListener<Callback>::CustomKeyListener(Callback callback): callback(callback){}
	
	template<typename Callback>
	void CustomKeyListener<Callback>::on_key_press(KeyPressEvent kpe)
	{
		this->callback(kpe);
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