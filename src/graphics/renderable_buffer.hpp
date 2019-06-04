//
// Created by Harrand on 04/06/2019.
//

#ifndef TOPAZ_RENDERABLE_BUFFER_HPP
#define TOPAZ_RENDERABLE_BUFFER_HPP
#include "platform_specific/mesh_buffer.hpp"
#include "platform_specific/shader_storage_buffer.hpp"
#include "graphics/render_pass.hpp"
#include <map>

// Assume each object has its own unique mesh. And no duplicate objects.

template<class ObjectType>
class RenderableBuffer
{
public:
    RenderableBuffer(GLuint layout_qualifier_id);
    std::size_t insert(ObjectType object);
    void render(RenderPass pass);
    static constexpr std::size_t capacity = 1024;
private:
    std::map<std::size_t, std::unique_ptr<ObjectType>> objects;
    tz::platform::OGLMeshBuffer mesh_buffer;
    tz::platform::OGLShaderStorageBuffer data_ssbo;
    MemoryPool<Matrix4x4> matrix_pool;
};

#include "graphics/renderable_buffer.inl"
#endif //TOPAZ_RENDERABLE_BUFFER_HPP