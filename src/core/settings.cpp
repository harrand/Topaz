#include "core/settings.hpp"
#include "core/tz_glad/glad_context.hpp"
#include "core/debug/assert.hpp"

namespace tz
{
    void RenderSettings::enable_wireframe_mode(bool wireframe) const
    {
        if(wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(1.0f);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    RenderSettings::CullTarget RenderSettings::get_culling() const
    {
        tz::RenderSettings::CullTarget cull;
        GLint val;
        glGetIntegerv(GL_CULL_FACE, &val);
        if(val == GL_FALSE)
        {
            cull = tz::RenderSettings::CullTarget::Nothing;
        }
        else
        {
            glGetIntegerv(GL_CULL_FACE_MODE, &val);
            switch(val)
            {
                case GL_BACK:
                    cull = tz::RenderSettings::CullTarget::BackFaces;
                break;
                case GL_FRONT:
                    cull = tz::RenderSettings::CullTarget::FrontFaces;
                break;
                case GL_FRONT_AND_BACK:
                    cull = tz::RenderSettings::CullTarget::Both;
                break;
                default:
                    topaz_assert(false, "Could not retrieve current culling mode. Perhaps this needs to be updated?");
                break;
            }
        }
        return cull;
    }

    void RenderSettings::set_culling(tz::RenderSettings::CullTarget culling) const
    {
        if(culling == tz::RenderSettings::CullTarget::Nothing)
        {
            glDisable(GL_CULL_FACE);
            return;
        }
        glEnable(GL_CULL_FACE);
        switch(culling)
        {
            case tz::RenderSettings::CullTarget::BackFaces:
                glCullFace(GL_BACK);
            break;
            case tz::RenderSettings::CullTarget::FrontFaces:
                glCullFace(GL_FRONT);
            break;
            case tz::RenderSettings::CullTarget::Both:
                glCullFace(GL_FRONT_AND_BACK);
            break;
            default:
                topaz_assert(false, "tz::RenderSettings::set_culling(CullTarget): Unknown CullTarget. Garbage enum value?");
            break;
        }
    }
}