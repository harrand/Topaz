#ifndef TOPAZ_GL_API_RENDER_PASS_HPP
#define TOPAZ_GL_API_RENDER_PASS_HPP
#include "gl/impl/common/render_pass_attachment.hpp"

namespace tz::gl
{
    /**
     * @brief Describes the subpasses, attachments and other parameters comprising a RenderPass.
     */
    class IRenderPassBuilder
    {
    public:
        virtual void add_pass(RenderPassAttachment attachment) = 0;
        virtual void finalise() = 0;
    };

    class IRenderPass
    {
    public:
        
    };

}

#endif // TOPAZ_GL_API_RENDER_PASS_HPP