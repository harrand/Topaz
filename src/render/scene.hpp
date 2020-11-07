#ifndef TOPAZ_RENDER_SCENE_HPP
#define TOPAZ_RENDER_SCENE_HPP
#include "render/asset.hpp"
#include "render/element_writer.hpp"
#include "gl/transform.hpp"
#include "gl/resource_writer.hpp"
#include "gl/camera.hpp"
#include <optional>

namespace tz::render
{
    /**
	 * \addtogroup tz_render Topaz Rendering Library (tz::render)
	 * High-level interface for 3D and 2D hardware-accelerated graphics programming. Used in combination with the \ref tz_gl "Topaz Graphics Library".
	 * @{
	 */

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

    namespace detail
    {
        template<class Element, class Writer>
        struct DefaultSceneElementWriterFunctor
        {
            void operator()(Writer& writer, const Element& element)
            {
                tz::render::ElementWriter<Element, Writer>::write(writer, element);
            }
        };
    }
    
    // Element must have operator() which returns its index snippet.
    /**
     * Represents a collection of assets, and scene elements.
     * @tparam Element The element type. It must have public data members available identical to that of tz::render::SceneElement. By default this is simply SceneElement.
     * @tparam Writer The writer type. Responsible for writing uniform data into a memory block on a per-element basis. The default is TransformResourceWriter.
     */
    template<class Element = SceneElement, class Writer = tz::gl::TransformResourceWriter, class SceneElementWriter = detail::DefaultSceneElementWriterFunctor<Element, Writer>>
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
         * Configures the device such that its next render invocation shall render all objects in the scene.
         * Note: The device's selected object, index-buffer and index-snippet-list will be changed.
         * @param device Device whose data should be updated.
         */
        void configure(tz::render::Device& device);
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
        /**
         * Retrieve the number of elements within the scene.
         * @return Number of elements within the scene.
         */
        std::size_t size() const;
        /**
         * Retrieve an iterator corresponding to the beginning of the element range.
         * @return Beginning of element range.
         */
        Iterator begin();
        /**
         * Retrieve an iterator corresponding to the end of the element range.
         * @return End of element range.
         */
        Iterator end();
        /**
         * Query as to whether an element exists within the scene such that `scene-element == param-element`
         * @param element Element to check is within the scene.
         * @return True if element is in the scene, otherwise false. 
         */
        bool contains(const Element& element) const;
        /**
         * Query as to whether an element exists within the scene corresponding to the given handle.
         * @param handle Handle to check whose element is within the scene.
         * @return True if the corresponding element is in the scene, otherwise false.
         */
        bool contains(Handle handle) const;
        /**
         * Erase the element from the scene, if it exists.
         * @param element Element to erase.
         * @return True if a deletion occurred, otherwise false.
         */
        bool erase(const Element& element);
        /**
         * Erase the element corresponding to the handle from the scene, if it exists.
         * @param handle Handle whose corresponding element should be erased.
         * @return True if a deletion occurred, otherwise false.
         */
        bool erase(Handle handle);
        /**
         * Remove all existing elements from the scene. This invalidates all existing handles.
         * Note: After clearing a scene, its size will be zero.
         */
        void clear();
        /**
         * Elements may become fragmented if many erasures happen, which may yield performance reductions. Packing the Scene ensures that elements are closely packed in memory.
         * Note: All handles and references become invalidated after packing. Do not pack the scene if you are caching handles.
         * Postcondition: If a cached handle is used after packing, it is not guaranteed to assert, but will certainly invoke UB.
         */
        void pack();
    private:
        std::vector<std::optional<Element>> elements = {};
        Writer writer;
    };
    /**
     * @}
     */
}

#include "render/scene.inl"
#endif // TOPAZ_RENDER_SCENE_HPP