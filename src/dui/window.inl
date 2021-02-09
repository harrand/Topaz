
namespace tz::ext::imgui
{
	template<class WindowType, typename... Args>
	WindowType& emplace_window(Args&&... args)
	{
		return static_cast<WindowType&>(add_window(std::make_unique<WindowType>(std::forward<Args>(args)...)));
	}
}