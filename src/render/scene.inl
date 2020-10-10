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
        return this->elements[handle];
    }

    template<class Element>
    Element& Scene<Element>::get(Handle handle)
    {
        return this->elements[handle];
    }

    template<class Element>
    void Scene<Element>::render(tz::render::Device& device)
    {
        tz::gl::IndexSnippetList indices;
        for(const Element& element : this->elements)
        {
            tz::render::AssetBuffer::Handle handle = element.mesh;
            const tz::render::MeshAsset& mesh = AssetBuffer::at(handle);
            indices.add_range(mesh());

            this->writer.write(element.pos, element.rot, element.scale, element.cam_pos, element.cam_rot, element.fov, element.aspect, element.near, element.far);
        }
        this->apply(device);
        device.set_indices(indices);
        device.render();
        this->writer.reset();
    }
}