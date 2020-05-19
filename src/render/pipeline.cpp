#include "render/pipeline.hpp"
#include "core/debug/assert.hpp"
#include <algorithm>

namespace tz::render
{
    Pipeline::Pipeline(): devices(){}

    std::size_t Pipeline::size() const
    {
        return this->devices.size();
    }

    bool Pipeline::empty() const
    {
        return this->devices.empty();
    }

    std::size_t Pipeline::add(Device d)
    {
        this->devices.push_back(d);
        return this->devices.size() - 1;
    }

    void Pipeline::erase(std::size_t idx)
    {
        topaz_assert(idx < this->size(), "tz::render::Pipeline::operator[", idx, "]: Out of range! Size: ", this->size());
        this->devices.erase(this->devices.begin() + idx);
    }

    void Pipeline::swap(std::size_t idx_a, std::size_t idx_b)
    {
        topaz_assert(idx_a < this->size(), "tz::render::Pipeline::swap(", idx_a, ", ", idx_b, "): Out of range (idx_a)! Size: ", this->size());
        topaz_assert(idx_b < this->size(), "tz::render::Pipeline::swap(", idx_a, ", ", idx_b, "): Out of range (idx_b)! Size: ", this->size());
        std::swap(this->devices[idx_a], this->devices[idx_b]);
    }

    const Device* Pipeline::operator[](std::size_t idx) const
    {
        topaz_assert(idx < this->size(), "tz::render::Pipeline::operator[", idx, "]: Out of range! Size: ", this->size());
        const auto& dev = this->devices[idx];
        if(dev == tz::render::Device::null_device())
            return nullptr;
        else
            return &dev;
    }

    Device* Pipeline::operator[](std::size_t idx)
    {
        topaz_assert(idx < this->size(), "tz::render::Pipeline::operator[", idx, "]: Out of range! Size: ", this->size());
        auto& dev = this->devices[idx];
        if(dev == tz::render::Device::null_device())
            return nullptr;
        else
            return &dev;
    }

    void Pipeline::render() const
    {
        for(const auto& device : this->devices)
        {
            if(!device.is_null())
                device.render();
        }
    }

    void Pipeline::clear() const
    {
        for(const auto& device : this->devices)
        {
            if(!device.is_null())
                device.clear();
        }
    }

    void Pipeline::purge()
    {
        this->devices.clear();
    }
}