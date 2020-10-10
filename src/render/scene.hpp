#ifndef TOPAZ_RENDER_SCENE_HPP
#define TOPAZ_RENDER_SCENE_HPP
#include "render/asset.hpp"

namespace tz::render
{
    enum class SceneData
    {
        Mesh,
        Position
    };

    struct SceneElement
    {
        SceneElement(tz::render::AssetBuffer::Handle handle): mesh(handle){}

        tz::Vec3 pos;
        tz::Vec3 rot;
        tz::Vec3 scale;
        tz::Vec3 cam_pos;
        tz::Vec3 cam_rot;
        float fov;
        float aspect;
        float near;
        float far;
        tz::render::AssetBuffer::Handle mesh;
    };
    // Element must have operator() which returns its index snippet.
    template<class Element = SceneElement>
    class Scene : public tz::render::AssetBuffer
    {
    public:
        using Handle = std::size_t;
        Scene(tz::mem::Block resource);
        void render(tz::render::Device& device);
        Handle add(Element element);
        const Element& get(Handle handle) const;
        Element& get(Handle handle);
    private:

        std::vector<Element> elements = {};
        tz::gl::TransformResourceWriter writer;
    };
}

#include "render/scene.inl"
#endif // TOPAZ_RENDER_SCENE_HPP