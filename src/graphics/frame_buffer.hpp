#ifndef TOPAZ_FRAME_BUFFER_HPP
#define TOPAZ_FRAME_BUFFER_HPP
#include "graphics/texture.hpp"

/**
* Simple wrapper for an OpenGL RenderBuffer. It's just a POD as they're write-only.
*/
class RenderBuffer
{
public:
    /**
     * Construct a RenderBuffer with all specifications.
     * @param width - Width of the RenderBuffer, in pixels
     * @param height - Height of the RenderBuffer, in pixels
     * @param internal_format - Internal OpenGL format of the RenderBuffer
     */
    RenderBuffer(int width, int height, GLenum internal_format = GL_RGBA);
    /**
    * OpenGL RenderBuffers are write-only, so cannot possibly read the data in which to copy or move.
    */
    RenderBuffer(const RenderBuffer& copy) = delete;
    /**
     * Construct a RenderBuffer from an existing RenderBuffer.
     * @param move - The RenderBuffer to move from.
     */
    RenderBuffer(RenderBuffer&& move);
    /**
     * Dispose of RenderBuffer data safely.
     */
    ~RenderBuffer();
    /**
    * RenderBuffer::operator= shall act like a pointer-copy; both share the same handle. However, when one dies the other becomes invalid, so this will be deleted too.
    */
    RenderBuffer& operator=(const RenderBuffer& rhs) = delete;
    friend class FrameBuffer;
private:
    /// Dimensions of the RenderBuffer, in pixels.
    const int width, height;
    /// Internal OpenGL format of the RenderBuffer.
    const GLenum internal_format;
    /// Underlying OpenGL render-buffer handle.
    GLuint renderbuffer_handle;
};

/**
* Something to draw to that isn't a window.
* FrameBuffer attachments can either be a Texture or a RenderBuffer.
*/
class FrameBuffer
{
public:
    /**
     * Construct a FrameBuffer with all specifications.
     * @param width - Width of the FrameBuffer, in pixels
     * @param height - Height of the FrameBuffer, in pixels
     */
    FrameBuffer(int width, int height);
    /**
     * Safely dispose of the FrameBuffer.
     */
    ~FrameBuffer();
    /**
     * Construct either a RenderBuffer or Texture in-place into this FrameBuffer.
     * @tparam Buffer - RenderBuffer or Texture
     * @tparam Args - The emplacement argument types
     * @param attachment - Which attachment the buffer should serve to provide
     * @param args - The emplacement arguments
     * @return - The constructed buffer
     */
    template<class Buffer, typename... Args>
    Buffer& emplace(GLenum attachment, Args&&... args);
    /**
     * Construct a Texture in-place into this FrameBuffer.
     * @tparam Args - The emplacement argument types
     * @param attachment - Which attachment the Texture should serve to provide
     * @param args - The emplacement arguments
     * @return - The constructed Texture
     */
    template<typename... Args>
    Texture& emplace_texture(GLenum attachment, Args&&... args);
    /**
     * Construct a RenderBuffer in-place into this FrameBuffer.
     * @tparam Args - The emplacement argument types
     * @param attachment - Which attachment the RenderBuffer should serve to provide
     * @param args - The emplacement arguments
     * @return - The constructed RenderBuffer
     */
    template<typename... Args>
    RenderBuffer& emplace_renderbuffer(GLenum attachment, Args&&... args);
    int get_width() const;
    int get_height() const;
    Vector2I get_dimensions() const;
    void set_width(int width);
    void set_height(int height);
    void set_dimensions(Vector2I dimensions);
    /**
    * Read-only access to all attachments to this FrameBuffer.
    * @return - Container of all attachments and the Texture/RenderBuffer serving the attachment
    */
    const std::unordered_map<GLenum, std::variant<Texture, RenderBuffer>>& get_attachments() const;
    /**
     * Read-only access to all texture-attachments to this FrameBuffer.
     * @return - Container of all Textures and their respective attachments
     */
    std::unordered_map<GLenum, std::reference_wrapper<const Texture>> get_texture_attachments() const;
    /**
     * Query whether this FrameBuffer is valid and ready to be rendered to.
     * @return - True if the FrameBuffer is valid. False otherwise
     */
    bool valid() const;
    /**
     * Query whether this FrameBuffer has a colour attachment.
     * @param attachment_index - Which colour index to query
     * @return - True if the specified colour attachment exists. False otherwise
     */
    bool has_colour(unsigned int attachment_index = 0) const;
    /**
     * Query whether this FrameBuffer has a depth attachment.
     * @return - True if there is a depth attachment. False otherwise
     */
    bool has_depth() const;
    /**
     * Query whether this FrameBuffer has a stencil attachment.
     * @return - True if there is a stencil attachment. False otherwise
     */
    bool has_stencil() const;
    /**
     * Specify which existing attachment should be connected to the output of the Fragment Shader.
     * @param attachment - The attachment to be given data.
     */
    void set_output_attachment(std::initializer_list<GLenum> attachments) const;
    /**
     * Perform an OpenGL clear operation.
     * @param mask - Mask of all desired buffer-bits
     * @param r - Red component of the clear-colour
     * @param g - Green component of the clear-colour
     * @param b - Blue component of the clear-colour
     * @param a - Alpha component of the clear-colour
     */
    void clear(tz::graphics::BufferBit buffer_bit, float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f) const;
    /**
     * Bind and sets the viewpoint to this framebuffer.
     * This means that any render calls will apply to this framebuffer.
     */
    void set_render_target() const;
private:
    /// Dimensions of the FrameBuffer.
    int width, height;
    /// Underlying OpenGL frame-buffer handle.
    GLuint framebuffer_handle;
    /// Container of all attachments and the Texture/RenderBuffer responsible for it.
    std::unordered_map<GLenum, std::variant<Texture, RenderBuffer>> attachments;
};

class ShadowMap : protected FrameBuffer
{
public:
    ShadowMap(int width, int height);
    const Texture& get_depth_texture() const;
    using FrameBuffer::set_render_target;
    using FrameBuffer::clear;
private:
    Texture& depth_texture;
};

#include "graphics/frame_buffer.inl"
#endif //TOPAZ_FRAME_BUFFER_HPP