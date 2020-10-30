#include "gl/index_snippet.hpp"
namespace tz::render
{
    template<class Element>
    Scene<Element>::Scene(tz::mem::Block resource): elements(), writer(resource)
    {

    }

    template<class Element>
    typename Scene<Element>::Handle Scene<Element>::add(Element element)
    {
        auto sz = this->elements.size();
        this->elements.push_back(element);
        return sz;
    }

    template<class Element>
    const Element& Scene<Element>::get(Handle handle) const
    {
        topaz_assert(handle < this->elements.size(), "tz::render::Scene<Element>::get(", handle, "): Handle out of range of element list!");
        return this->elements[handle];
    }

    template<class Element>
    Element& Scene<Element>::get(Handle handle)
    {
        topaz_assert(handle < this->elements.size(), "tz::render::Scene<Element>::get(", handle, "): Handle out of range of element list!");
        return this->elements[handle];
    }

    template<class Element>
    std::size_t Scene<Element>::size() const
    {
        return this->elements.size();
    }

    template<class Element>
    typename Scene<Element>::Iterator Scene<Element>::begin()
    {
        return this->elements.begin();
    }

    template<class Element>
    typename Scene<Element>::Iterator Scene<Element>::end()
    {
        return this->elements.end();
    }

    template<class Element>
    void Scene<Element>::render(tz::render::Device& device)
    {
        // Write mesh and element data to the resource, then tell it to render.
        tz::gl::IndexSnippetList indices;
        for(const Element& element : this->elements)
        {
            if(!element.visible)
            {
                continue;
            }
            tz::render::AssetBuffer::Index idx = element.mesh;
            const tz::render::MeshAsset& mesh = AssetBuffer::at(idx);
            indices.add_range(mesh());

            this->writer.write(element.transform.position, element.transform.rotation, element.transform.scale, element.camera.position, element.camera.rotation, element.camera.fov, element.camera.aspect_ratio, element.camera.near, element.camera.far);
        }
        this->apply(device);
        device.set_indices(indices);
        device.render();
        this->writer.reset();
    }

    template<class Element>
    bool Scene<Element>::contains(const Element& element) const
    {
        return std::find(this->elements.begin(), this->elements.end(), element) != this->elements.end();
    }

    template<class Element>
    bool Scene<Element>::contains(Handle handle) const
    {
        return (this->size() > handle);
    }
}