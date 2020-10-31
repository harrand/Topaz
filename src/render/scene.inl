#include "gl/index_snippet.hpp"
#include <numeric>

namespace tz::render
{
    template<class Element>
    Scene<Element>::Iterator::Iterator(std::optional<Element>* iter, const std::optional<Element>* const end): iter(iter), end(end){}
    template<class Element>
    bool Scene<Element>::Iterator::operator==(const Iterator& rhs) const
    {
        return this->iter == rhs.iter;
    }

    template<class Element>
    bool Scene<Element>::Iterator::operator!=(const Iterator& rhs) const
    {
        return this->iter != rhs.iter;
    }

    template<class Element>
    const Element& Scene<Element>::Iterator::operator*() const
    {
        topaz_assert(this->iter->has_value(), "Scene<Element>::Iterator::operator*() yielded nullopt. This should've been skipped over.");
        return this->iter->value();
    }
    template<class Element>
    Element& Scene<Element>::Iterator::operator*()
    {
        topaz_assert(this->iter->has_value(), "Scene<Element>::Iterator::operator*() yielded nullopt. This should've been skipped over.");
        return this->iter->value();
    }
    template<class Element>
    typename Scene<Element>::Iterator& Scene<Element>::Iterator::operator++()
    {
        do
        {
            ++(this->iter);
        }while(!this->iter->has_value() && (this->iter > this->end));
        return *this;
    }
    template<class Element>
    typename Scene<Element>::Iterator Scene<Element>::Iterator::operator++(int)
    {
        auto iter_cpy = this->iter;
        do
        {
            this->iter++;
        }while(!this->iter->has_value() && (this->iter > this->end));
        return {iter_cpy, this->end};
    }
    template<class Element>
    typename Scene<Element>::Iterator& Scene<Element>::Iterator::operator--()
    {
        do
        {
            --(this->iter);
        }while(!this->iter->has_value());
        return *this;
    }
    template<class Element>
    typename Scene<Element>::Iterator Scene<Element>::Iterator::operator--(int)
    {
        auto iter_cpy = this->iter;
        do
        {
            this->iter--;
        }while(!this->iter->has_value());
        return {iter_cpy, this->end};
    }
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
        topaz_assert(this->elements[handle].has_value(), "tz::render::Scene<Element>::get(", handle, "): Element at index is nullopt.");
        return this->elements[handle].value();
    }

    template<class Element>
    Element& Scene<Element>::get(Handle handle)
    {
        topaz_assert(handle < this->elements.size(), "tz::render::Scene<Element>::get(", handle, "): Handle out of range of element list!");
        topaz_assert(this->elements[handle].has_value(), "tz::render::Scene<Element>::get(", handle, "): Element at index is nullopt.");
        return this->elements[handle].value();
    }

    template<class Element>
    std::size_t Scene<Element>::size() const
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

    template<class Element>
    typename Scene<Element>::Iterator Scene<Element>::begin()
    {
        return {this->elements.data(), &(this->elements.back())};
    }

    template<class Element>
    typename Scene<Element>::Iterator Scene<Element>::end()
    {
        return {&(this->elements.back()), &(this->elements.back())};
    }

    template<class Element>
    void Scene<Element>::render(tz::render::Device& device)
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
        return std::find(this->elements.begin(), this->elements.end(), std::optional<Element>{element}) != this->elements.end();
    }

    template<class Element>
    bool Scene<Element>::contains(Handle handle) const
    {
        return (this->elements.size() > handle) && this->elements[handle].has_value();
    }

    template<class Element>
    bool Scene<Element>::erase(const Element& element)
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

    template<class Element>
    bool Scene<Element>::erase(Handle handle)
    {
        if(!this->elements[handle].has_value() || handle >= this->elements.size())
        {
            return false;
        }
        return this->erase(this->get(handle));
    }

    template<class Element>
    void Scene<Element>::pack()
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