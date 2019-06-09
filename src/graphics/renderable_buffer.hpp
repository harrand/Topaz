//
// Created by Harrand on 04/06/2019.
//

#ifndef TOPAZ_RENDERABLE_BUFFER_HPP
#define TOPAZ_RENDERABLE_BUFFER_HPP
#include "platform_specific/mesh_buffer.hpp"
#include "platform_specific/shader_storage_buffer.hpp"
#include "graphics/render_pass.hpp"
#include "graphics/renderable.hpp"
#include <map>

// Assume each object has its own unique mesh. And no duplicate objects.
// This limit should be removed after the proof-of-concept is complete.

template<class ObjectType>
class RenderableBuffer
{
public:
    RenderableBuffer(GLuint layout_qualifier_id);
    RenderableBuffer(const RenderableBuffer& copy);
    RenderableBuffer(RenderableBuffer<ObjectType>&& move);
    RenderableBuffer<ObjectType>& operator=(RenderableBuffer<ObjectType>&& rhs);
    std::size_t insert(ObjectType object);
    template<class ObjectSubType, typename... Args>
    ObjectSubType& emplace(Args&&... args);
    void render(RenderPass pass) const;
    static constexpr std::size_t capacity = 1024;
    friend class Scene;
private:
    Renderable* take_object(std::unique_ptr<Renderable> object);

    std::map<std::size_t, std::unique_ptr<ObjectType>> objects;
    tz::platform::OGLMeshBuffer mesh_buffer;
    tz::platform::OGLShaderStorageBuffer data_ssbo;
    mutable MemoryPool<Matrix4x4> matrix_pool;
};

#include "graphics/renderable_buffer.inl"
#endif //TOPAZ_RENDERABLE_BUFFER_HPP