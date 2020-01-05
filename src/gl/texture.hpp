#ifndef TOPAZ_GL_TEXTURE_HPP
#define TOPAZ_GL_TEXTURE_HPP
#include "gl/image.hpp"
#include <optional>

namespace tz::gl
{
    using TextureHandle = GLuint;

    struct TextureParameters;

    struct TextureDataDescriptor
    {
        GLenum component_type;
        GLint internal_format;
        GLenum format;
    
        unsigned int width;
        unsigned int height;
        /// Note: Excludes width and height, only formatting data.
        bool operator==(const TextureDataDescriptor& rhs) const;
        bool operator!=(const TextureDataDescriptor& rhs) const;
    };

    class Texture
    {
    public:
        Texture();
        virtual ~Texture();
        void set_parameters(const TextureParameters& params);
        std::size_t size() const;
        bool empty() const;
        unsigned int get_width() const;
        unsigned int get_height() const;
        void bind(std::size_t binding_id) const;
        template<template<typename> class PixelType, typename ComponentType>
        void set_data(const tz::gl::Image<PixelType<ComponentType>>& image);
        template<template<typename> class PixelType, typename ComponentType>
        tz::gl::Image<PixelType<ComponentType>> get_data() const;
    protected:
        void internal_bind() const;
        void internal_unbind() const;

        TextureHandle handle;
        std::optional<TextureDataDescriptor> descriptor;
    };

    enum class TextureMinificationFunction : GLint
    {
        Nearest = GL_NEAREST,
        WeightedAverage = GL_LINEAR,
        NearestClosestMinmap = GL_NEAREST_MIPMAP_NEAREST,
        WeightedAverageClosestMinmap = GL_LINEAR_MIPMAP_NEAREST,
        NearestAverageOfMinmaps = GL_NEAREST_MIPMAP_LINEAR,
        WeightedAverageAverageOfMinmaps = GL_LINEAR_MIPMAP_LINEAR,
    };

    enum class TextureMagnificationFunction : GLint
    {
        Nearest = GL_NEAREST,
        WeightedAverage = GL_LINEAR,
    };

    // s, t == u, v
    enum class TextureWrapFunction : GLint
    {
        ClampToEdge = GL_CLAMP_TO_EDGE,
        MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE,
        ClampToBorder = GL_CLAMP_TO_BORDER,
        MirroredRepeat = GL_MIRRORED_REPEAT,
        Repeat = GL_REPEAT,
    };

    struct TextureParameters
    {
        TextureMinificationFunction min_filter;
        TextureMagnificationFunction mag_filter;
        TextureWrapFunction horizontal_wrap;
        TextureWrapFunction vertical_wrap;
    };

    /// For those times where you really don't care about life.
    static constexpr TextureParameters default_texture_params{TextureMinificationFunction::Nearest, TextureMagnificationFunction::Nearest, TextureWrapFunction::Repeat, TextureWrapFunction::Repeat};
}

#include "gl/texture.inl"
#endif // TOPAZ_GL_TEXTURE_HPP