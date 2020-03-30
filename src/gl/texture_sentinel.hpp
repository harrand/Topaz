#ifndef TOPAZ_GL_TEXTURE_SENTINEL_HPP
#define TOPAZ_GL_TEXTURE_SENTINEL_HPP
#include <vector>
#include <glad/glad.h>

namespace tz::gl
{
    // GL_ARB_bindless_texture is an incredibly dangerous extension, so the sentinel is designed to minimise hazards should the feature ever be misused.
    // Note: Terminal textures make use of GL_ARB_bindless_texture
    // Note: TextureSentinels are stubbed out in release.
    class TextureSentinel
    {
    public:
        TextureSentinel() = default;
        void register_handle(GLuint64 handle);
        void make_resident(GLuint64 handle);
        std::size_t registered_handle_count() const;
        GLuint64 get_handle(std::size_t id) const;
        bool registered(GLuint64 handle) const;
        bool resident(GLuint64 handle) const;
        bool ready(GLuint64 handle) const;
        void notify_usage(GLuint64 handle) const;
    private:
    #if TOPAZ_DEBUG
        std::vector<GLuint64> declared;
        std::vector<GLuint64> residents;
    #endif
    };
}
#endif // TOPAZ_GL_TEXTURE_SENTINEL_HPP