//#include <format> TODO: Compiler support for std::format

namespace img2cpp
{
    template<tz::gl::PixelType PixelT>
    constexpr std::string serialise_cpp(const tz::gl::Image<PixelT>& image)
    {
        unsigned int num_pixels = image.get_width() * image.get_height();
        constexpr std::size_t one_component_size = sizeof(typename PixelT::ComponentType);
        constexpr std::size_t one_pixel_size = PixelT::num_components * one_component_size;
        unsigned long long image_size_bytes = one_pixel_size * num_pixels;
        // TODO: Compiler support for std::format
        //std::string result = std::format("constexpr std::array<std::byte, {}> image_data{", image_size_bytes);
        std::string result = std::string("constexpr std::array<std::byte, ") + std::to_string(image_size_bytes) + "> image_data{";
        {
            // Fill data.
            const PixelT* img_data_pixels = image.data();
            const auto* img_data_bytes = reinterpret_cast<const std::byte*>(img_data_pixels);
            for(unsigned long long i = 0; i < image_size_bytes; i++)
            {
                const std::byte& b = img_data_bytes[i];
                result += std::string{"static_cast<std::byte>("} + std::to_string(static_cast<int>(b)) + ")";
                if(i < (image_size_bytes) - 1)
                {
                    result += ",";
                }
            }
        }
        result += "};";
        return result;
    }
}