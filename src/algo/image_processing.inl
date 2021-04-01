namespace tz::algo
{
    template<tz::gl::PixelType Pixel>
    void flip_image_horizontally(tz::gl::Image<Pixel>& image)
    {
        for(std::size_t x = 0; x < image.get_width() / 2; x++)
        {
            for(std::size_t y = 0; y < image.get_height(); y++)
            {
                std::swap(image(x, y), image(image.get_width() - 1 - x, y));
            }
        }
    }

    template<tz::gl::PixelType Pixel>
    void flip_image_vertically(tz::gl::Image<Pixel>& image)
    {
        for(std::size_t x = 0; x < image.get_width(); x++)
        {
            for(std::size_t y = 0; y < image.get_height() / 2; y++)
            {
                std::swap(image(x, y), image(x, image.get_height() - 1 - y));
            }
        }
    }

    template<tz::gl::PixelType Pixel>
    void flip_image(tz::gl::Image<Pixel>& image, ImageFlipDirection direction)
    {
        switch(direction)
        {
            case ImageFlipDirection::Horizontal:
                flip_image_horizontally(image);
            break;
            case ImageFlipDirection::Vertical:
                flip_image_vertically(image);
            break;
            default:
                topaz_assert(false, "");
            break;
        }
    }
}