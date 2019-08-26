//
// Created by Harrand on 25/08/2019.
//

#include "font.hpp"
#include "utility/log.hpp"
#include <map>

FT_Library tz::graphics::detail::freetype_library = {};
bool tz::graphics::detail::freetype_initialized = false;

Font::Font(const std::string& font_path, int pixel_height): font_path(font_path), pixel_height(pixel_height), font_handle()
{
    topaz_assert(tz::graphics::detail::freetype_initialized, "Font::Font(...): Attempted to create font before tz::graphics was initialised! This must never happen.");
    auto error = FT_New_Face(tz::graphics::detail::freetype_library, this->font_path.c_str(), 0, &this->font_handle);
    topaz_assert(error != FT_Err_Unknown_File_Format, "Font::Font(filename, pixel_height): Font file provided could be opened and read, but is unsupported. Use another!");
    topaz_assert(error == 0, "Font::Font(filename, pixel_height): Failed to read the font file \"", font_path, "\" properly. Perhaps the path is incorrect or there is a permissions issue?");
    FT_Set_Pixel_Sizes(this->font_handle, 0, static_cast<FT_UInt>(pixel_height));
}

Font::Font(const Font& copy): Font(copy.font_path, copy.pixel_height){}
Font::Font(Font&& move): font_path(move.font_path), pixel_height(move.pixel_height), font_handle(move.font_handle)
{
    move.font_handle = nullptr;
}

Font::~Font()
{
    if(this->font_handle == nullptr) // if its been moved, dont try and delete it'll crash if you do
        return;
    FT_Done_Face(this->font_handle);
    this->font_handle = nullptr;
}

Font& Font::operator=(Font rhs)
{
    std::swap(this->font_path, rhs.font_path);
    std::swap(this->pixel_height, rhs.pixel_height);
    std::swap(this->font_handle, rhs.font_handle);
    return *this;
}

Font& Font::operator=(Font&& rhs)
{
    this->font_path = rhs.font_path;
    this->pixel_height = rhs.pixel_height;
    this->font_handle = rhs.font_handle;
    rhs.font_handle = nullptr;
    return *this;
}

int Font::get_pixel_height() const
{
    return this->pixel_height;
}

const std::string& Font::get_path() const
{
    return this->font_path;
}

Image Font::render_bitmap(const std::string& text) const
{
    auto image_vector = this->get_image_sequence(text);
    std::vector<std::byte> resultant_data;
    topaz_assert(!image_vector.empty(), "Texture::Texture(font, text): Font");
    std::map<const Image*, std::vector<std::vector<std::byte>>> image_to_rows_map;
    unsigned int width = 0, height = 0;
    for(const Image& image : image_vector)
    {
        width += image.get_width();
        height = std::max(image.get_height(), height);
        for(std::size_t i = 0 ; i < image.get_height(); i++)
        {
            std::vector<std::byte> row;
            row.resize(image.get_width() * 4);
            for(std::size_t j = 0; j < image.get_width() * 4; j++)
            {
                std::size_t index = (i * image.get_width() * 4) + j;
                row[j] = image.data()[index];
            }
            image_to_rows_map[&image].push_back(std::move(row));
        }
    }
    std::size_t max_row_count = 0;
    for(auto const& pair : image_to_rows_map)
    {
        max_row_count = std::max(max_row_count, pair.second.size());
    }

    auto append_data = [](std::vector<std::byte>& data, const std::vector<std::byte>& to_append)
    {
        for(const auto& byte : to_append)
            data.push_back(byte);
    };

    for(std::size_t i = 0; i < max_row_count; i++)
    {
        for(const Image& image : image_vector)
        {
            try
            {
                append_data(resultant_data, image_to_rows_map[&image].at(i));
            }catch(...)
            {
                // pad with zeros if there's no data left for this image (smaller than the others);
                for([[maybe_unused]] std::size_t j = 0; j < image.get_width(); j++)
                {
                    // red for example purposes.
                    resultant_data.push_back(std::byte{0});
                    resultant_data.push_back(std::byte{255});
                    resultant_data.push_back(std::byte{0});
                    resultant_data.push_back(std::byte{255});
                }
            }
        }
    }
    topaz_assert(resultant_data.size() == static_cast<std::size_t>(width * height * 4), "Texture::Texture(Font, ...): Expected data size to be ", width, "*", height, "*4 == ", (width * height * 4), ", but the size was ", resultant_data.size());
    return {resultant_data, width, height};
}

std::vector<Image> Font::get_image_sequence(const std::string& text) const
{
    std::vector<Image> image_sequence;
    for(char c : text)
    {
        if(FT_Load_Char(this->font_handle, c, FT_LOAD_RENDER))
        {
            tz::debug::print("Font::render_bitmap(text): Failed to load glyph from character '", c, "'");
            continue;
        }

        unsigned int width = this->font_handle->glyph->bitmap.width;
        unsigned int height = this->font_handle->glyph->bitmap.rows;
        // the bitmap now contains an 8-bit grayscale image. That's fine, we can easily convert that to RGBA8888.
        unsigned char* bitmap_data = this->font_handle->glyph->bitmap.buffer;
        std::vector<std::byte> image_data;
        image_data.reserve(width * height * 4);
        for(std::size_t i = 0; i < (width * height); i++)
        {
            std::byte data_element{bitmap_data[i]};
            image_data.push_back(data_element);
            image_data.push_back(data_element);
            image_data.push_back(data_element);
            image_data.push_back(data_element);
        }
        image_sequence.emplace_back(image_data, width, height);
    }
    return image_sequence;
}