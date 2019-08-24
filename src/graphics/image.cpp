//
// Created by Harrand on 24/08/2019.
//

#include "image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

Image::Image(std::string path): path({path}), data(), width(0), height(0)
{
    int w, h, comps;
    unsigned char* stbi_data = stbi_load(this->path.value().c_str(), &w, &h, &comps, 4);
    this->width = static_cast<unsigned int>(w);
    this->height = static_cast<unsigned int>(h);
    topaz_assert(stbi_data != nullptr, "Image::Image(path): Need to be passed a file which actually contains image data! The path \"", this->path.value(), "\" does not.");
    auto data_size = static_cast<std::size_t>(this->width * this->height * 4);
    this->data.resize(data_size);
    for(std::size_t i = 0; i < data_size; i++)
    {
        this->data[i] = static_cast<std::byte>(stbi_data[i]);
    }
    stbi_image_free(stbi_data);
}

Image::Image(const aiTexture* assimp_texture): path({std::nullopt}), data(), width(0), height(0)
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
        this->data.reserve(this->width * this->height);
        // guaranteed to be a multiple of 4
        for(std::size_t i = 3; i <= (4 * this->width * this->height); i += 4)
        {
            //bitmap.pixels.emplace_back(PixelRGBA{image_data[i - 3], image_data[i - 2], image_data[i - 1], image_data[i]});
            this->data.push_back(static_cast<std::byte>(image_data[i - 3]));
            this->data.push_back(static_cast<std::byte>(image_data[i - 2]));
            this->data.push_back(static_cast<std::byte>(image_data[i - 1]));
            this->data.push_back(static_cast<std::byte>(image_data[i]));
        }
        stbi_image_free(image_data);
        tz::debug::print("Image::Image(aiTexture*): number of pixels in the ", this->width, "x", this->height, " assimp texture = ", this->data.size(), "\n");
    }
    else
    {
        this->width = assimp_texture->mWidth;
        this->height = assimp_texture->mHeight;
        for (std::size_t i = 0; i < this->width * this->height; i++)
        {
            const aiTexel &texel = assimp_texture->pcData[i];
            this->data.push_back(static_cast<std::byte>(texel.r));
            this->data.push_back(static_cast<std::byte>(texel.g));
            this->data.push_back(static_cast<std::byte>(texel.b));
            this->data.push_back(static_cast<std::byte>(texel.a));
            //bitmap.pixels.emplace_back(texel.r, texel.g, texel.b, texel.a);
        }
    }
}

const MemoryPool<std::byte> Image::get_data() const
{
    return {const_cast<std::vector<std::byte>&>(this->data)};
}

MemoryPool<std::byte> Image::get_data()
{
    return {this->data.data(), this->width * this->height};
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