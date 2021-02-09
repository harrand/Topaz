namespace tz
{
	template<typename WindowType, typename... Args>
	std::size_t TopazCore::emplace_extra_window(Args&&... args)
	{
		std::unique_ptr<IWindow>& window_ptr = this->secondary_windows.emplace_back(std::make_unique<WindowType>(std::forward<Args>(args)...));
		window_ptr->register_this();
		auto& window = static_cast<WindowType&>(*window_ptr);
		return this->secondary_windows.size() - 1;
	}
}