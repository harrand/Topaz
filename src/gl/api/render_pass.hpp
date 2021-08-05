#ifndef TOPAZ_GL_API_RENDER_PASS_HPP
#define TOPAZ_GL_API_RENDER_PASS_HPP
#include "gl/impl/frontend/common/render_pass_attachment.hpp"

namespace tz::gl
{
    /**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * A collection of low-level renderer-agnostic graphical interfaces.
	 * @{
	 */
    /**
     * @brief Describes the subpasses, attachments and other parameters comprising a RenderPass.
     */
    class IRenderPassBuilder
    {
    public:
        virtual void add_pass(RenderPassAttachment attachment) = 0;
        virtual void set_presentable_output(bool presentable_output) = 0;
        virtual bool has_presentable_output() const = 0;
    };

    /**
     * @brief Render-passes describe the composition of a single render-invocation of a @ref IRenderer. See @ref IRenderer::render().
     */
    class IRenderPass
    {
    public:
        virtual bool requires_depth_image() const = 0;
    };
    /**
     * @}
     */
}

#endif // TOPAZ_GL_API_RENDER_PASS_HPP