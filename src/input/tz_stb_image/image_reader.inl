#include "core/core.hpp"
#include "core/resource_manager.hpp"
#include "core/debug/assert.hpp"

namespace tz::ext::stb
{
    template<class PixelType>
    tz::gl::Image<PixelType> read_image(const char* path)
    {
        constexpr std::size_t num_desired_components = PixelType::num_components;
        int channels_in_file;
        int w, h;
        unsigned char* data = stbi_load((tz::core::res().get_path() + path).c_str(), &w, &h, &channels_in_file, num_desired_components);
        topaz_assert(data != nullptr, "tz::ext::stb::read_image(", path, "): Could not load image data from memory.");
        auto width = static_cast<std::size_t>(w);
        auto height = static_cast<std::size_t>(h);
        std::size_t data_size = width * height * num_desired_components;
        tz::gl::Image<PixelType> res{static_cast<unsigned int>(width), static_cast<unsigned int>(height)};
        for(std::size_t i = 0; i < (width * height) - 1; i++)
        {
            PixelType& pix = res.data()[i];
            const std::size_t cur_pixel_offset = i * num_desired_components;
            for(std::size_t j = 0; j < static_cast<std::size_t>(num_desired_components); j++)
            {
                topaz_assert(cur_pixel_offset + j < data_size, "tz::ext::stb::read_image(", path, "): Invalid pixel data offset: ", cur_pixel_offset + j, " must be less than the data size (", data_size, ")");
                pix[j] = static_cast<std::byte>(data[cur_pixel_offset + j]);
            }
        }
        return res;
    }
}