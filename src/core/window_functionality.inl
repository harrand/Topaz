namespace tz
{
	void WindowFunctionality::add_resize_callback(WindowResizeConcept auto on_window_resize)
	{
		this->window_resize_callbacks.push_back(on_window_resize);
	}

}