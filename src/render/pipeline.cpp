#include "render/pipeline.hpp"
#include "core/debug/assert.hpp"

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
    }

    void Pipeline::erase(std::size_t idx)
    {
        topaz_assert(idx < this->size(), "tz::render::Pipeline::operator[", idx, "]: Out of range! Size: ", this->size());
        this->devices[idx] = std::nullopt;
    }

    const Device* Pipeline::operator[](std::size_t idx) const
    {
        topaz_assert(idx < this->size(), "tz::render::Pipeline::operator[", idx, "]: Out of range! Size: ", this->size());
        const auto& dev = this->devices[idx];
        if(dev.has_value())
            return &dev.value();
        return nullptr;
    }

    Device* Pipeline::operator[](std::size_t idx)
    {
        topaz_assert(idx < this->size(), "tz::render::Pipeline::operator[", idx, "]: Out of range! Size: ", this->size());
        auto& dev = this->devices[idx];
        if(dev.has_value())
            return &dev.value();
        return nullptr;
    }

    void Pipeline::render() const
    {
        for(const auto& device : this->devices)
        {
            if(device.has_value())
                device->render();
        }
    }

    void Pipeline::clear() const
    {
        for(const auto& device : this->devices)
        {
            if(device.has_value())
                device->clear();
        }
    }
}