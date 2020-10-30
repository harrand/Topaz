#ifndef TOPAZ_RENDER_SCENE_HPP
#define TOPAZ_RENDER_SCENE_HPP
#include "render/asset.hpp"
#include "gl/transform.hpp"
#include "gl/resource_writer.hpp"
#include "gl/camera.hpp"

namespace tz::render
{
    struct SceneElement
    {
        /**
         * Construct a SceneElement using the given assetbuffer index.
         */
        SceneElement(tz::render::AssetBuffer::Index idx): mesh(idx){}
        tz::gl::Transform get_transform() const;
        tz::gl::CameraData get_camera_data() const;
        tz::render::AssetBuffer::Index get_mesh_index() const;

        /// Transform of the element, in world-space.
        tz::gl::Transform transform = {};
        /// Camera data representing the viewer, in world-space.
        tz::gl::CameraData camera = {};
        /// AssetBuffer index representing the attached mesh.
        tz::render::AssetBuffer::Index mesh;
        bool visible = true;
    };
    // Element must have operator() which returns its index snippet.
    /**
     * Represents a collection of assets, and scene elements.
     * @tparam Element The element type. It must have public data members available identical to that of tz::render::SceneElement. By default this is simply SceneElement.
     */
    template<class Element = SceneElement>
    class Scene : public tz::render::AssetBuffer
    {
    public:
        using Handle = std::size_t;
        using Iterator = typename std::vector<Element>::iterator;
        /**
         * Constructs an empty scene. Element data is written into the given resource.
         * @param resource Memory block where element data is written into. It is recommended for this to be some mapped buffer data.
         */
        Scene(tz::mem::Block resource);
        /**
         * Renders all elements within the scene using the given device. Invisible elements will be skipped.
         * Note: The device's selected object, index-buffer and index-snippet-list will be changed.
         * @param device Device whose existing data shall be used.
         */
        void render(tz::render::Device& device);
        /**
         * Add a scene element.
         * Precondition: The Element's mesh asset must have previously been registered via add_mesh(index). Otherwise, this will invoke UB without asserting on the next render(Device&) invocation.
         * @param element Scene element to add.
         * @return Handle referencing the newly-added element. Use Scene::get(handle) to retrieve this element.
         */
        Handle add(Element element);
        /**
         * Retrieve an element which was previously added to the scene.
         * Precondition: handle corresponds to a valid handle retrieved by a past invocation of Scene<Element>::add(...). Otherwise, this will assert and invoke UB.
         * 
         * @param handle Handle to an existing scene element to retrieve.
         * @return Const reference to the corresponding handle.
         */
        const Element& get(Handle handle) const;
        /**
         * Retrieve an element which was previously added to the scene.
         * Precondition: handle corresponds to a valid handle retrieved by a past invocation of Scene<Element>::add(...). Otherwise, this will assert and invoke UB.
         * 
         * @param handle Handle to an existing scene element to retrieve.
         * @return Reference to the corresponding handle.
         */
        Element& get(Handle handle);
        std::size_t size() const;
        Iterator begin();
        Iterator end();
        bool contains(const Element& element) const;
        bool contains(Handle handle) const;
    private:
        std::vector<Element> elements = {};
        tz::gl::TransformResourceWriter writer;
    };
}

#include "render/scene.inl"
#endif // TOPAZ_RENDER_SCENE_HPP