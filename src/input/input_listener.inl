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
}