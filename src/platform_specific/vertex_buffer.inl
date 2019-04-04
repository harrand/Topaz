#ifdef TOPAZ_OPENGL
namespace tz::platform
{
    template<template<typename> typename Container, typename POD>
    void OGLVertexBuffer::insert(const Container<POD>& data, const OGLVertexBufferUsage& usage) const
    {
        this->bind();
        std::size_t size = data.size() * tz::utility::generic::sizeof_element(data);
        glNamedBufferData(this->vbo_handle, size, data.data(), usage());
    }

    template<template<typename> typename Container, typename POD>
    std::optional<Container<POD>> OGLVertexBuffer::query_all_data() const
    {
        std::size_t size = this->get_size();
        if(size == 0) // If there is no data stored, return null.
            return std::nullopt;
        Container<POD> data;
        // Number of elements = size in bytes / bytes per object
        data.resize(size / sizeof(POD));
        glGetNamedBufferSubData(this->vbo_handle, 0, size, data.data());
        return {data};
    }

    template<typename T>
    void OGLVertexAttribute::define(GLint size, GLboolean normalised, GLsizei stride) const
    {
        using namespace tz::platform;
        if constexpr(std::is_same_v<T, short>)
        {
            this->direct_define(size, OGLVertexAttributeType::SHORT, normalised, stride);
        }
        else if constexpr(std::is_same_v<T, unsigned short>)
        {
            this->direct_define(size, OGLVertexAttributeType::USHORT, normalised, stride);
        }
        else if constexpr(std::is_same_v<T, int>)
        {
            this->direct_define(size, OGLVertexAttributeType::INT, normalised, stride);
        }
        else if constexpr(std::is_same_v<T, unsigned int>)
        {
            this->direct_define(size, OGLVertexAttributeType::UINT, normalised, stride);
        }
        else if constexpr(std::is_same_v<T, float>)
        {
            this->direct_define(size, OGLVertexAttributeType::FLOAT, normalised, stride);
        }
        else if constexpr(std::is_same_v<T, double>)
        {
            this->direct_define(size, OGLVertexAttributeType::DOUBLE, normalised, stride);
        }
        else
        {
            static_assert(std::is_void_v<T>, "OGLVertexAttribute::define<T>(std::size_t): T is not a valid type. Accepted types are short, ushort, int, uint, float and double.");
        }
    }

    template<typename... Args>
    OGLVertexBuffer& OGLVertexArray::emplace_vertex_buffer(Args&&... args)
    {
        this->bind();
        this->vertex_buffers.push_back(std::make_unique<OGLVertexBuffer>(std::forward<Args>(args)...));
        return *(this->vertex_buffers.back().get());
    }

    template<typename... Args>
    OGLVertexAttribute& OGLVertexArray::emplace_vertex_attribute(Args&&... args)
    {
        this->bind();
        this->vertex_attributes.push_back(std::make_unique<OGLVertexAttribute>(std::forward<Args>(args)...));
        return *(this->vertex_attributes.back().get());
    }
}
#endif