#ifndef TOPAZ_RENDER_SCENE_HPP
#define TOPAZ_RENDER_SCENE_HPP
#include "render/asset.hpp"
#include "gl/transform.hpp"
#include "gl/resource_writer.hpp"
#include "gl/camera.hpp"
#include <optional>

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
        class Iterator
        {
        public:
            Iterator(std::optional<Element>* iter, const std::optional<Element>* const end);
            bool operator==(const Iterator& rhs) const;
            bool operator!=(const Iterator& rhs) const;
            const Element& operator*() const;
            Element& operator*();
            Iterator& operator++();
            Iterator operator++(int);
            Iterator& operator--();
            Iterator operator--(int);
        private:
            std::optional<Element>* iter;
            const std::optional<Element>* const end;
        };
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
         * Note: Do *not* cache this reference. References are not stable and can be invalidated at any time.
         * 
         * @param handle Handle to an existing scene element to retrieve.
         * @return Const reference to the corresponding handle.
         */
        const Element& get(Handle handle) const;
        /**
         * Retrieve an element which was previously added to the scene.
         * Precondition: handle corresponds to a valid handle retrieved by a past invocation of Scene<Element>::add(...). Otherwise, this will assert and invoke UB.
         * Note: Do *not* cache this reference. References are not stable and can be invalidated at any time.
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
        bool erase(const Element& element);
        bool erase(Handle handle);
        /**
         * Elements may become fragmented if many erasures happen, which may yield performance reductions. Packing the Scene ensures that elements are closely packed in memory.
         * Note: All handles and references become invalidated after packing. Do not pack the scene if you are caching handles.
         * Postcondition: If a cached handle is used after packing, it is not guaranteed to assert, but will certainly invoke UB.
         */
        void pack();
    private:
        std::vector<std::optional<Element>> elements = {};
        tz::gl::TransformResourceWriter writer;
    };
}

#include "render/scene.inl"
#endif // TOPAZ_RENDER_SCENE_HPP