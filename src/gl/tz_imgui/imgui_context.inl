#include <vector>
#include <memory>

namespace tz::ext::imgui
{
    static std::vector<std::unique_ptr<ImGuiWindow>> windows;

    template<class WindowType, typename... Args>
    WindowType& emplace_window(Args&&... args)
    {
        windows.push_back(std::make_unique<WindowType>(std::forward<Args>(args)...));
        return *(windows.back());
    }
}