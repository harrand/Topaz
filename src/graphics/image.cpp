//
// Created by Harrand on 24/08/2019.
//

#include "image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

Image::Image(std::string path): path({path}), image_data(), width(0), height(0)
{
    int w, h, comps;
    unsigned char* stbi_data = stbi_load(this->path.value().c_str(), &w, &h, &comps, 4);
    this->width = static_cast<unsigned int>(w);
    this->height = static_cast<unsigned int>(h);
    topaz_assert(stbi_data != nullptr, "Image::Image(path): Need to be passed a file which actually contains image data! The path \"", this->path.value(), "\" does not.");
    auto data_size = static_cast<std::size_t>(this->width * this->height * 4ull);
    this->image_data.resize(data_size);
    for(std::size_t i = 0; i < data_size; i++)
    {
        this->image_data[i] = static_cast<std::byte>(stbi_data[i]);
    }
    stbi_image_free(stbi_data);
}

Image::Image(const aiTexture* assimp_texture): path({std::nullopt}), image_data(), width(0), height(0)
{
    if(assimp_texture->mHeight == 0 && assimp_texture->mWidth != 0)
    {
        // texture is compressed
        unsigned int compressed_data_size = assimp_texture->mWidth;
        int comps;
        int w, h;
        stbi_uc* image_data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(assimp_texture->pcData), compressed_data_size, &w, &h, &comps, STBI_rgb_alpha);
        this->width = static_cast<unsigned int>(w);
        this->height = static_cast<unsigned int>(h);
        this->image_data.reserve(this->width * this->height * 4);
        // guaranteed to be a multiple of 4
        for(std::size_t i = 3; i <= (4 * this->width * this->height); i += 4)
        {
            this->image_data.push_back(static_cast<std::byte>(image_data[i - 3]));
            this->image_data.push_back(static_cast<std::byte>(image_data[i - 2]));
            this->image_data.push_back(static_cast<std::byte>(image_data[i - 1]));
            this->image_data.push_back(static_cast<std::byte>(image_data[i]));
        }
        stbi_image_free(image_data);
        tz::debug::print("Image::Image(aiTexture*): number of pixels in the ", this->width, "x", this->height, " assimp texture = ", this->image_data.size(), "\n");
    }
    else
    {
        this->width = assimp_texture->mWidth;
        this->height = assimp_texture->mHeight;
        for (std::size_t i = 0; i < this->width * this->height; i++)
        {
            const aiTexel &texel = assimp_texture->pcData[i];
            this->image_data.push_back(static_cast<std::byte>(texel.r));
            this->image_data.push_back(static_cast<std::byte>(texel.g));
            this->image_data.push_back(static_cast<std::byte>(texel.b));
            this->image_data.push_back(static_cast<std::byte>(texel.a));
        }
    }
}

Image::Image(std::vector<std::byte> image_data, unsigned int width, unsigned int height): path({std::nullopt}), image_data(std::move(image_data)), width(width), height(height)
{
    topaz_assert(this->image_data.size() == (this->width * this->height * 4), "Image::Image(pixel data): Number of image data elements provided (", this->image_data.size(), ") is incorrect. It should be ", this->width, "*", this->height, "*4 == ", (this->width * this->height * 4));
}

const BytePool Image::data() const
{
    return {const_cast<std::vector<std::byte>&>(this->image_data)};
}

BytePool Image::data()
{
    return {this->image_data};
}

Vector2UI Image::get_dimensions() const
{
    return {this->width, this->height};
}

unsigned int Image::get_width() const
{
    return this->width;
}

unsigned int Image::get_height() const
{
    return this->height;
}

std::size_t Image::get_size() const
{
    return this->image_data.size();
}

bool Image::has_filename() const
{
    return this->path.has_value();
}

const std::string* Image::get_filename() const
{
    if(this->has_filename())
        return &this->path.value();
    return nullptr;
}