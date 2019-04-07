//
// Created by Harrand on 04/04/2019.
//

#include "vertex_buffer.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
    OGLVertexBufferUsage::OGLVertexBufferUsage(OGLVertexBufferFrequency frequency, OGLVertexBufferNature nature): frequency(frequency), nature(nature) {}

    OGLVertexBufferUsage::OGLVertexBufferUsage(const GLenum& usage): frequency(OGLVertexBufferFrequency::STATIC), nature(OGLVertexBufferNature::DRAW)
    {
        switch(usage)
        {
            default:
            case GL_STREAM_DRAW:
                this->frequency = OGLVertexBufferFrequency::STREAM;
                this->nature = OGLVertexBufferNature::DRAW;
                break;
            case GL_STREAM_READ:
                this->frequency = OGLVertexBufferFrequency::STREAM;
                this->nature = OGLVertexBufferNature::READ;
                break;
            case GL_STREAM_COPY:
                this->frequency = OGLVertexBufferFrequency::STREAM;
                this->nature = OGLVertexBufferNature::COPY;
                break;
            case GL_STATIC_DRAW:
                this->frequency = OGLVertexBufferFrequency::STATIC;
                this->nature = OGLVertexBufferNature::DRAW;
                break;
            case GL_STATIC_READ:
                this->frequency = OGLVertexBufferFrequency::STATIC;
                this->nature = OGLVertexBufferNature::READ;
                break;
            case GL_STATIC_COPY:
                this->frequency = OGLVertexBufferFrequency::STATIC;
                this->nature = OGLVertexBufferNature::COPY;
                break;
            case GL_DYNAMIC_DRAW:
                this->frequency = OGLVertexBufferFrequency::DYNAMIC;
                this->nature = OGLVertexBufferNature::DRAW;
                break;
            case GL_DYNAMIC_READ:
                this->frequency = OGLVertexBufferFrequency::DYNAMIC;
                this->nature = OGLVertexBufferNature::READ;
                break;
            case GL_DYNAMIC_COPY:
                this->frequency = OGLVertexBufferFrequency::DYNAMIC;
                this->nature = OGLVertexBufferNature::COPY;
                break;
        }
    }

    GLenum OGLVertexBufferUsage::operator()() const
    {
        // Nested switches, basically just multiplexing
        using namespace tz::platform;
        switch(this->frequency)
        {
        case OGLVertexBufferFrequency::STREAM:
            switch(this->nature)
            {
                case OGLVertexBufferNature::DRAW:
                    return GL_STREAM_DRAW;
                case OGLVertexBufferNature::READ:
                    return GL_STREAM_READ;
                case OGLVertexBufferNature::COPY:
                    return GL_STREAM_COPY;
            }
        case OGLVertexBufferFrequency::STATIC:
            switch(this->nature)
            {
                case OGLVertexBufferNature::DRAW:
                    return GL_STATIC_DRAW;
                case OGLVertexBufferNature::READ:
                    return GL_STATIC_READ;
                case OGLVertexBufferNature::COPY:
                    return GL_STATIC_COPY;
            }
        case OGLVertexBufferFrequency::DYNAMIC:
            switch(this->nature)
            {
                case OGLVertexBufferNature::DRAW:
                    return GL_DYNAMIC_DRAW;
                case OGLVertexBufferNature::READ:
                    return GL_DYNAMIC_READ;
                case OGLVertexBufferNature::COPY:
                    return GL_DYNAMIC_COPY;
            }
        }
        // If the enums are ill-formed, default to static draw.
        return GL_STATIC_DRAW;
    }

    OGLVertexBuffer::OGLVertexBuffer(OGLVertexBufferTarget target): target(target), vbo_handle(0)
    {
        glGenBuffers(1, &this->vbo_handle);
    }

    OGLVertexBuffer::OGLVertexBuffer(const OGLVertexBuffer& copy): OGLVertexBuffer(copy.target)
    {
        std::optional<std::vector<std::byte>> generic_data = copy.query_all_data<std::vector, std::byte>();
        std::optional<OGLVertexBufferUsage> generic_usage = copy.query_current_usage();
        if(generic_data.has_value() && generic_usage.has_value())
        {
            this->insert(generic_data.value(), generic_usage.value());
        }
    }

    OGLVertexBuffer::OGLVertexBuffer(OGLVertexBuffer&& move): target(move.target), vbo_handle(move.vbo_handle)
    {
        move.vbo_handle = 0;
    }

    OGLVertexBuffer::~OGLVertexBuffer()
    {
        glDeleteBuffers(1, &this->vbo_handle);
    }

    OGLVertexBuffer& OGLVertexBuffer::operator=(OGLVertexBuffer rhs)
    {
        OGLVertexBuffer::swap(*this, rhs);
        return *this;
    }

    const OGLVertexBufferTarget& OGLVertexBuffer::get_target() const
    {
        return this->target;
    }

    std::size_t OGLVertexBuffer::get_size() const
    {
        GLint size;
        glGetNamedBufferParameteriv(this->vbo_handle, GL_BUFFER_SIZE, &size);
        return static_cast<std::size_t>(size);
    }

    bool OGLVertexBuffer::empty() const
    {
        return this->get_size() == 0;
    }

    void OGLVertexBuffer::allocate_memory(std::size_t size, const OGLVertexBufferUsage& usage) const
    {
        glNamedBufferData(this->vbo_handle, size, nullptr, usage());
    }

    void OGLVertexBuffer::update(GLintptr offset, GLsizeiptr size, const void* data) const
    {
        glNamedBufferSubData(this->vbo_handle, offset, size, data);
    }

    std::optional<OGLVertexBufferUsage> OGLVertexBuffer::query_current_usage() const
    {
        if(this->empty())
            return std::nullopt;
        GLint usage;
        glGetNamedBufferParameteriv(this->vbo_handle, GL_BUFFER_USAGE, &usage);
        return {OGLVertexBufferUsage{static_cast<const GLenum>(usage)}};
    }

    void OGLVertexBuffer::bind() const
    {
        glBindBuffer(static_cast<GLenum>(this->target), this->vbo_handle);
    }

    void OGLVertexBuffer::unbind() const
    {
        glBindBuffer(static_cast<GLenum>(this->target), 0);
    }

    void OGLVertexBuffer::swap(OGLVertexBuffer& lhs, OGLVertexBuffer& rhs)
    {
        std::swap(lhs.vbo_handle, rhs.vbo_handle);
        std::swap(lhs.target, rhs.target);
    }

    OGLVertexTransformFeedbackBuffer::OGLVertexTransformFeedbackBuffer(OGLVertexBufferTarget target, GLuint output_id): OGLVertexBuffer(target), output_id(output_id){}

    void OGLVertexTransformFeedbackBuffer::bind() const
    {
        OGLVertexBuffer::bind();
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, this->output_id, this->vbo_handle);
    }

    OGLVertexAttribute::OGLVertexAttribute(GLuint attribute_id): attribute_id(attribute_id){}

    GLuint OGLVertexAttribute::get_id() const
    {
        return this->attribute_id;
    }

    void OGLVertexAttribute::enable() const
    {
        glEnableVertexAttribArray(this->attribute_id);
    }

    void OGLVertexAttribute::disable() const
    {
        glDisableVertexAttribArray(this->attribute_id);
    }

    void OGLVertexAttribute::direct_define(GLint size, const OGLVertexAttributeType& type, GLboolean normalised, GLsizei stride) const
    {
        glVertexAttribPointer(this->attribute_id, size, static_cast<GLenum>(type), normalised, stride, NULL);
    }

    void OGLVertexAttribute::instanced_define(GLuint divisor) const
    {
        glVertexAttribDivisor(this->attribute_id, divisor);
    }

    OGLVertexArray::OGLVertexArray(): vao_handle(0), vertex_buffers(), vertex_attributes()
    {
        glGenVertexArrays(1, &this->vao_handle);
    }

    OGLVertexArray::OGLVertexArray(const OGLVertexArray& copy): OGLVertexArray()
    {
        // Emplace copies of all attributes and buffers.
        for(const std::unique_ptr<OGLVertexAttribute>& attrib_ptr : copy.vertex_attributes)
            this->emplace_vertex_attribute(*attrib_ptr);
        for(const std::unique_ptr<OGLVertexBuffer>& buffer_ptr : copy.vertex_buffers)
            this->emplace_vertex_buffer(*buffer_ptr);
    }

    OGLVertexArray::OGLVertexArray(OGLVertexArray&& move): vao_handle(move.vao_handle), vertex_buffers(std::move(move.vertex_buffers)), vertex_attributes(std::move(move.vertex_attributes))
    {
        move.vao_handle = 0;
    }

    OGLVertexArray::~OGLVertexArray()
    {
        // "Unused names in arrays are silently ignored, as is the value zero." - OGL Specification
        glDeleteVertexArrays(1, &this->vao_handle);
    }

    OGLVertexArray& OGLVertexArray::operator=(OGLVertexArray rhs)
    {
        std::swap(*this, rhs);
        return *this;
    }

    void OGLVertexArray::bind() const
    {
        glBindVertexArray(this->vao_handle);
    }

    void OGLVertexArray::unbind() const
    {
        glBindVertexArray(0);
    }

    const OGLVertexAttribute* OGLVertexArray::get_attribute(GLuint attribute_id) const
    {
        for(const std::unique_ptr<OGLVertexAttribute>& attrib_ptr : this->vertex_attributes)
            if(attrib_ptr->get_id() == attribute_id)
                return attrib_ptr.get();
        return nullptr;
    }

    const OGLVertexBuffer* OGLVertexArray::get_element_array_buffer() const
    {
        for(const std::unique_ptr<OGLVertexBuffer>& buffer_ptr : this->vertex_buffers)
            if(buffer_ptr->get_target() == OGLVertexBufferTarget::ELEMENT_ARRAY)
                return buffer_ptr.get();
        return nullptr;
    }

    bool OGLVertexArray::operator==(const OGLVertexArray& rhs) const
    {
        return this->vao_handle == rhs.vao_handle;
    }

    void OGLVertexArray::swap(OGLVertexArray& lhs, OGLVertexArray& rhs)
    {
        std::swap(lhs.vao_handle, rhs.vao_handle);
        std::swap(lhs.vertex_buffers, rhs.vertex_buffers);
        std::swap(lhs.vertex_attributes, rhs.vertex_attributes);
    }
}
#endif