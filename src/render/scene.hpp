#ifndef TOPAZ_RENDER_SCENE_HPP
#define TOPAZ_RENDER_SCENE_HPP
#include "render/asset.hpp"
#include "gl/transform.hpp"
#include "gl/camera.hpp"

namespace tz::render
{
    enum class SceneData
    {
        Mesh,
        Position
    };

    struct SceneElement
    {
        SceneElement(tz::render::AssetBuffer::Index idx): mesh(idx){}

        tz::gl::Transform transform = {};
        tz::gl::CameraData camera = {};
        tz::render::AssetBuffer::Index mesh;
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