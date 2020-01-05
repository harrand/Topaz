#ifndef TOPAZ_RENDER_PIPELINE_HPP
#define TOPAZ_RENDER_PIPELINE_HPP
#include "render/device.hpp"
#include <vector>
#include <optional>

namespace tz::render
{
    /**
     * Represents at least zero render-passes.
     * 
     * Can be used to neatly abstract away complicated render-pass sequences.
     * Example:
     * - 0: Device to render the scene into a frame (outputs to texture 'tex')
     * - 1: Device to render the texture 'tex' into the window frame (Using a post-processing shader)
     * Pipelines can have devices added to them. Clearing/Rendering a pipeline will invoke the same operation on all devices in chronological order.
     */
    class Pipeline
    {
    public:
        /**
         * Construct an empty pipeline with no devices.
         */
        Pipeline();
        /**
         * Retrieve the number of devices in the pipeline.
         * 
         * Note: This includes null devices.
         * @return Number of contained devices.
         */
        std::size_t size() const;
        /**
         * Query as to whether there are any devices in the pipeline.
         * 
         * Note: This includes null devices.
         * @return True if there are no devices in the pipeline. False otherwise.
         */
        bool empty() const;
        /**
         * Add a device to the end of the pipeline.
         * 
         * @param d Device to render/clear after all others.
         * @return Index corresponding to the new device.
         */
        std::size_t add(Device d);
        /**
         * Remove a device at the given index, preventing subsequent Clear/Render invocations from using it.
         * 
         * Precondition: The index must be in-range (idx < this->size()). Otherwise, this will assert and invoke UB.
         * @param idx Index corresponding to the device to erase.
         */
        void erase(std::size_t idx);
        /**
         * Retrieve a pointer to the device at the given index.
         * 
         * Precondition: The index must be in-range (idx < this->size()). Otherwise, this will assert and invoke UB.
         * @param idx Index corresponding to the device to retrieve.
         * @return Immutable pointer to the device at 'idx' if it is not a null-device. Otherwise nullptr.
         */
        const Device* operator[](std::size_t idx) const;
        /**
         * Retrieve a pointer to the device at the given index.
         * 
         * Precondition: The index must be in-range (idx < this->size()). Otherwise, this will assert and invoke UB.
         * @param idx Index corresponding to the device to retrieve.
         * @return Mutable pointer to the device at 'idx' if it is not a null-device. Otherwise nullptr.
         */
        Device* operator[](std::size_t idx);
        /**
         * Render all contained non-null devices in chronological order.
         */
        void render() const;
        /**
         * Clear all contained non-null devices in chronological order.
         */
        void clear() const;
    private:
        std::vector<std::optional<Device>> devices;
    };
}

#endif // TOPAZ_RENDER_PIPELINE_HPP