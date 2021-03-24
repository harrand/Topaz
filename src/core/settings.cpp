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

    RenderSettings::DepthTesting RenderSettings::get_depth_testing() const
    {
        GLint val;
        glGetIntegerv(GL_DEPTH_TEST, &val);
        if(val == GL_FALSE)
        {
            return DepthTesting::AlwaysPass;
        }
        glGetIntegerv(GL_DEPTH_FUNC, &val);
        switch(val)
        {
            case GL_NEVER:
                return DepthTesting::NeverPass;
            break;
            case GL_LESS:
                return DepthTesting::PassIfLess;
            break;
            case GL_EQUAL:
                return DepthTesting::PassIfEqual;
            break;
            case GL_LEQUAL:
                return DepthTesting::PassIfLequal;
            break;
            case GL_GREATER:
                return DepthTesting::PassIfGreater;
            break;
            case GL_NOTEQUAL:
                return DepthTesting::PassIfNequal;
            break;
            case GL_GEQUAL:
                return DepthTesting::PassIfGequal;
            break;
            case GL_ALWAYS:
                return DepthTesting::AlwaysPass;
            break;
            default:
                topaz_assert(false, "tz::RenderSettings::get_depth_testing(): Unexpected state. Returning default of DepthTesting::PassIfLess.");
                return DepthTesting::PassIfLess;
            break;
        }
    }

    void RenderSettings::set_depth_testing(DepthTesting depth_testing)
    {
        if(depth_testing == DepthTesting::AlwaysPass)
        {
            glDisable(GL_DEPTH_TEST);
        }
        else
        {
            glEnable(GL_DEPTH_TEST);
            switch(depth_testing)
            {
                case DepthTesting::NeverPass:
                    glDepthFunc(GL_NEVER);
                break;
                case DepthTesting::PassIfLess:
                    glDepthFunc(GL_LESS);
                break;
                case DepthTesting::PassIfEqual:
                    glDepthFunc(GL_EQUAL);
                break;
                case DepthTesting::PassIfLequal:
                    glDepthFunc(GL_LEQUAL);
                break;
                case DepthTesting::PassIfGreater:
                    glDepthFunc(GL_GREATER);
                break;
                case DepthTesting::PassIfNequal:
                    glDepthFunc(GL_NOTEQUAL);
                break;
                case DepthTesting::PassIfGequal:
                    glDepthFunc(GL_GEQUAL);
                break;
                case DepthTesting::AlwaysPass:
                    glDepthFunc(GL_ALWAYS);
                break;
                default:
                    topaz_assert(false, "tz::RenderSettings::set_depth_testing(DepthTesting): Invalid state. Using default of DepthTesting::PassIfLess.");
                    glDepthFunc(GL_LESS);
                break;
            }
        }
    }
}