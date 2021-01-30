#include "gl/index_snippet.hpp"
#include <numeric>

namespace tz::render
{
    template<>
    class ElementWriter<SceneElement, tz::gl::TransformResourceWriter>
    {
    public:
        static void write(tz::gl::TransformResourceWriter& writer, const SceneElement& element)
        {
            writer.write(element.transform.position, element.transform.rotation, element.transform.scale, element.camera.position, element.camera.rotation, element.camera.fov, element.camera.aspect_ratio, element.camera.near, element.camera.far);
        }
    };
    
    template<class Element, class Writer, class SceneElementWriter>
    Scene<Element, Writer, SceneElementWriter>::Iterator::Iterator(std::optional<Element>* iter, const std::optional<Element>* const end): iter(iter), end(end){}
    template<class Element, class Writer, class SceneElementWriter>
    bool Scene<Element, Writer, SceneElementWriter>::Iterator::operator==(const Iterator& rhs) const
    {
        return this->iter == rhs.iter;
    }

    template<class Element, class Writer, class SceneElementWriter>
    bool Scene<Element, Writer, SceneElementWriter>::Iterator::operator!=(const Iterator& rhs) const
    {
        return this->iter != rhs.iter;
    }

    template<class Element, class Writer, class SceneElementWriter>
    const Element& Scene<Element, Writer, SceneElementWriter>::Iterator::operator*() const
    {
        topaz_assert(this->iter->has_value(), "Scene<Element, Writer, SceneElementWriter>::Iterator::operator*() yielded nullopt. This should've been skipped over.");
        return this->iter->value();
    }
    template<class Element, class Writer, class SceneElementWriter>
    Element& Scene<Element, Writer, SceneElementWriter>::Iterator::operator*()
    {
        topaz_assert(this->iter->has_value(), "Scene<Element, Writer, SceneElementWriter>::Iterator::operator*() yielded nullopt. This should've been skipped over.");
        return this->iter->value();
    }
    template<class Element, class Writer, class SceneElementWriter>
    typename Scene<Element, Writer, SceneElementWriter>::Iterator& Scene<Element, Writer, SceneElementWriter>::Iterator::operator++()
    {
        do
        {
            ++(this->iter);
        }while(!this->iter->has_value() && (this->iter > this->end));
        return *this;
    }
    template<class Element, class Writer, class SceneElementWriter>
    typename Scene<Element, Writer, SceneElementWriter>::Iterator Scene<Element, Writer, SceneElementWriter>::Iterator::operator++(int)
    {
        auto iter_cpy = this->iter;
        do
        {
            this->iter++;
        }while(!this->iter->has_value() && (this->iter > this->end));
        return {iter_cpy, this->end};
    }
    template<class Element, class Writer, class SceneElementWriter>
    typename Scene<Element, Writer, SceneElementWriter>::Iterator& Scene<Element, Writer, SceneElementWriter>::Iterator::operator--()
    {
        do
        {
            --(this->iter);
        }while(!this->iter->has_value());
        return *this;
    }
    template<class Element, class Writer, class SceneElementWriter>
    typename Scene<Element, Writer, SceneElementWriter>::Iterator Scene<Element, Writer, SceneElementWriter>::Iterator::operator--(int)
    {
        auto iter_cpy = this->iter;
        do
        {
            this->iter--;
        }while(!this->iter->has_value());
        return {iter_cpy, this->end};
    }
    template<class Element, class Writer, class SceneElementWriter>
    Scene<Element, Writer, SceneElementWriter>::Scene(tz::mem::Block resource): elements(), writer(resource)
    {

    }

    template<class Element, class Writer, class SceneElementWriter>
    typename Scene<Element, Writer, SceneElementWriter>::Handle Scene<Element, Writer, SceneElementWriter>::add(Element element)
    {
        auto sz = this->elements.size();
        this->elements.push_back(element);
        return sz;
    }

    template<class Element, class Writer, class SceneElementWriter>
    const Element& Scene<Element, Writer, SceneElementWriter>::get(Handle handle) const
    {
        topaz_assert(handle < this->elements.size(), "tz::render::Scene<Element, Writer, SceneElementWriter>::get(", handle, "): Handle out of range of element list!");
        topaz_assert(this->elements[handle].has_value(), "tz::render::Scene<Element, Writer, SceneElementWriter>::get(", handle, "): Element at index is nullopt.");
        return this->elements[handle].value();
    }

    template<class Element, class Writer, class SceneElementWriter>
    Element& Scene<Element, Writer, SceneElementWriter>::get(Handle handle)
    {
        topaz_assert(handle < this->elements.size(), "tz::render::Scene<Element, Writer, SceneElementWriter>::get(", handle, "): Handle out of range of element list!");
        topaz_assert(this->elements[handle].has_value(), "tz::render::Scene<Element, Writer, SceneElementWriter>::get(", handle, "): Element at index is nullopt.");
        return this->elements[handle].value();
    }

    template<class Element, class Writer, class SceneElementWriter>
    std::size_t Scene<Element, Writer, SceneElementWriter>::size() const
    {
        
        auto add_if = [](std::size_t cur_total, const std::optional<Element>& cur_ele)
        {
            if(cur_ele.has_value())
            {
                return cur_total + 1;
            }
            return cur_total;
        };
        return std::accumulate(this->elements.begin(), this->elements.end(), 0, add_if);        
    }

    template<class Element, class Writer, class SceneElementWriter>
    typename Scene<Element, Writer, SceneElementWriter>::Iterator Scene<Element, Writer, SceneElementWriter>::begin()
    {
        return {this->elements.data(), &(this->elements.back()) + 1};
    }

    template<class Element, class Writer, class SceneElementWriter>
    typename Scene<Element, Writer, SceneElementWriter>::Iterator Scene<Element, Writer, SceneElementWriter>::end()
    {
        return {&(this->elements.back()) + 1, &(this->elements.back()) + 1};
    }

    template<class Element, class Writer, class SceneElementWriter>
    void Scene<Element, Writer, SceneElementWriter>::configure(tz::render::Device& device)
    {
        // Write mesh and element data to the resource, then tell it to render.
        tz::gl::IndexSnippetList indices;
        for(const std::optional<Element>& ele : this->elements)
        {
            if(!ele.has_value())
            {
                continue;
            }
            const Element& element = ele.value();
            tz::render::AssetBuffer::Index idx = element.get_mesh_index();
            const tz::render::MeshAsset& mesh = AssetBuffer::at(idx);
            indices.add_range(mesh());

            SceneElementWriter{}(this->writer, element);
        }
        this->apply(device);
        device.set_indices(indices);
        this->writer.reset();
    }

    template<class Element, class Writer, class SceneElementWriter>
    bool Scene<Element, Writer, SceneElementWriter>::contains(const Element& element) const
    {
        return std::find(this->elements.begin(), this->elements.end(), std::optional<Element>{element}) != this->elements.end();
    }

    template<class Element, class Writer, class SceneElementWriter>
    bool Scene<Element, Writer, SceneElementWriter>::contains(Handle handle) const
    {
        return (this->elements.size() > handle) && this->elements[handle].has_value();
    }

    template<class Element, class Writer, class SceneElementWriter>
    bool Scene<Element, Writer, SceneElementWriter>::erase(const Element& element)
    {
        bool erased_some = false;
        for(std::optional<Element>& ele : this->elements)
        {
            if(ele.has_value() && ele.value() == element)
            {
                ele = std::nullopt;
                erased_some = true;
            }
        }
        return erased_some;
    }

    template<class Element, class Writer, class SceneElementWriter>
    bool Scene<Element, Writer, SceneElementWriter>::erase(Handle handle)
    {
        if(!this->elements[handle].has_value() || handle >= this->elements.size())
        {
            return false;
        }
        return this->erase(this->get(handle));
    }

    template<class Element, class Writer, class SceneElementWriter>
    void Scene<Element, Writer, SceneElementWriter>::clear()
    {
        this->elements.clear();
    }

    template<class Element, class Writer, class SceneElementWriter>
    void Scene<Element, Writer, SceneElementWriter>::pack()
    {
        for(auto iter = this->elements.begin(); iter != this->elements.end();)
        {
            if(iter->has_value())
            {
                ++iter;
            }
            else
            {
                iter = this->elements.erase(iter);
            }
        }
    }
}