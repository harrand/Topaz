#if TZ_VULKAN
#include <utility>

namespace tz::gl::vk
{
    template<typename... Args>
    RenderPassBuilder& RenderPassBuilder::with(Args&&... args)
    {
        this->subpasses.emplace_back(*this, std::forward<Args>(args)...);
        return *this;
    }
}

#endif // TZ_VULKAN