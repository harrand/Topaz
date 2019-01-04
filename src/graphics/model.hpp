//
// Created by Harry on 03/01/2019.
//

#ifndef TOPAZ_MODEL_HPP
#define TOPAZ_MODEL_HPP

#include "graphics/mesh.hpp"
#include "graphics/texture.hpp"
#include "assimp/material.h"
#include "graphics/shader.hpp"

class Model
{
public:
    Model(std::string filename);
    std::size_t get_number_of_meshes() const;
    virtual void render(Shader* shader, GLenum mode = GL_TRIANGLES) const;
private:
    static std::vector<std::string> load_material_texture_references(aiMaterial* material, aiTextureType type);
    std::vector<Mesh> meshes;
    std::multimap<unsigned int, Texture> material_textures;
};

#endif //TOPAZ_MODEL_HPP
