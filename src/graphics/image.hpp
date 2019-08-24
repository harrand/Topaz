//
// Created by Harrand on 24/08/2019.
//

#ifndef TOPAZ_IMAGE_HPP
#define TOPAZ_IMAGE_HPP
#include "utility/memory.hpp"
#include "assimp/texture.h"
#include <string>
#include <cstddef>
#include <vector>

// Wrapper for an image loaded from the filesystem.
// Always RGBA8888.
class Image
{
public:
    Image(std::string path);
    Image(const aiTexture* assimp_texture);
    const MemoryPool<std::byte> get_data() const;
    MemoryPool<std::byte> get_data();
    Vector2UI get_dimensions() const;
    unsigned int get_width() const;
    unsigned int get_height() const;
private:
    std::optional<std::string> path;
    std::vector<std::byte> data;
    unsigned int width, height;
};


#endif //TOPAZ_IMAGE_HPP
