#ifndef TOPAZ_GL_API_RENDER_PASS_HPP
#define TOPAZ_GL_API_RENDER_PASS_HPP
#include "gl/impl/common/render_pass_attachment.hpp"

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
    };

    /**
     * @brief Render-passes describe the composition of a single render-invocation of a @ref IRenderer. See @ref IRenderer::render().
     */
    class IRenderPass
    {
    public:
        
    };
    /**
     * @}
     */
}

#endif // TOPAZ_GL_API_RENDER_PASS_HPP