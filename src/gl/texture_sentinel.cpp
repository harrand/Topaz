#include "gl/texture_sentinel.hpp"
#include "core/debug/assert.hpp"
#include <algorithm>

namespace tz::gl
{
    void TextureSentinel::register_handle(GLuint64 handle)
    {
        #if TOPAZ_DEBUG
            topaz_hard_assert(!this->registered(handle), "tz::gl::TextureSentinel::register_handle(handle): Handle already has been registered!");
            this->declared.push_back(handle);
        #endif
    }

    void TextureSentinel::make_resident(GLuint64 handle)
    {
        #if TOPAZ_DEBUG
            topaz_hard_assert(this->registered(handle), "tz::gl::TextureSentinel::make_resident(handle): Handle has not yet been registered!");
            this->residents.push_back(handle);
        #endif
    }

    bool TextureSentinel::registered(GLuint64 handle) const
    {
        #if TOPAZ_DEBUG
            return std::find(this->declared.begin(), this->declared.end(), handle) != this->declared.end();
        #else
            return false;
        #endif
    }

    bool TextureSentinel::resident(GLuint64 handle) const
    {
        #if TOPAZ_DEBUG
            return std::find(this->residents.begin(), this->residents.end(), handle) != this->residents.end();
        #else
            return false;
        #endif
    }

    bool TextureSentinel::ready(GLuint64 handle) const
    {
        return this->registered(handle) && this->resident(handle);
    }

    void TextureSentinel::notify_usage(GLuint64 handle) const
    {
        #if TOPAZ_DEBUG
            topaz_hard_assert(this->ready(handle), "tz::gl::TextureSentinel::notify_usage(handle): Bindless handle is not ready!");
        #endif
    }
}