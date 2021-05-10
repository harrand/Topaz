#if TZ_OGL
#include "gl/impl/ogl/device.hpp"

namespace tz::gl
{
    RenderPass DeviceOGL::create_render_pass(RenderPassBuilder builder) const
    {
        return {builder};
    }
}
#endif // TZ_OGL