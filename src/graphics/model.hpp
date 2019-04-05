//
// Created by Harry on 03/01/2019.
//

#ifndef TOPAZ_MODEL_HPP
#define TOPAZ_MODEL_HPP

#include "graphics/mesh.hpp"
#include "graphics/texture.hpp"
#include "assimp/material.h"
#include "graphics/shader.hpp"
#include <map>

class Model
{
public:
    /**
     * Constructs a model given a path to a supported model file.
     * Supported file formats include FBX, OBJ, COLLADA (.dae), 3DS, DirectX X, Wavefront OBJ and Blender 3D (.blend) and more, upto a total of 57 file formats.
     * An exhaustive list for supported formats is available here: http://assimp.sourceforge.net/main_features_formats.html
     * @param filename - Path to the model file
     */
    Model(std::string filename);
    /**
     * Get the number of meshes imported from the model file.
     * @return - Number of meshes in the model, after optimisations
     */
    std::size_t get_number_of_meshes() const;
    /**
     * Get a mesh corresponding to a given ID.
     * @param mesh_id - ID of the mesh to retrieve
     * @return - Pointer to the corresponding mesh if it exists. If no such mesh exists with the given ID, nullptr is returned
     */
    const Mesh* get_mesh_by_id(std::size_t mesh_id) const;
    /**
     * Get the number of textures imported from materials from the model file.
     * @return - Number of textures in the model.
     */
    std::size_t get_number_of_textures() const;
    /**
     * For a given mesh ID, return a container of constant references to all textures which are required by the given mesh.
     * @param mesh_id - ID of the mesh whose corresponding textures need be retrieved
     * @return - Vector of constant texture references corresponding to the given mesh. If no mesh exists with the given ID, the container will be empty
     */
    std::vector<std::reference_wrapper<const Texture>> get_textures_in_mesh(std::size_t mesh_id) const;
    /**
     * Render all meshes in the model using all required textures.
     * Models imported from external files are always triangulated.
     * @param shader - Shader with which to render the model
     * @param mode - Render primitive mode with which to render the model. This is triangular by default. As imported Models are always triangular, this should remain its default value unless non-triangular models are constructed manually
     */
    virtual void render(Shader& shader) const;
private:
    /**
     * For a given assimp material, extract all filesystem paths referencing corresponding textures comprising the material.
     * @param material - Material whose textures should be queried
     * @param type - The type of textures desired. This is used to filter the textures retrieved
     * @return - Container of filesystem paths referencing existing textures to import
     */
    static std::vector<std::string> load_material_texture_references(aiMaterial* material, aiTextureType type);
    /// Container of all meshes comprising the model.
    std::vector<Mesh> meshes;
    /// Map of mesh IDs and all their corresponding extracted textures. Textures can be subclasses too, such as NormalMaps.
    std::multimap<unsigned int, std::unique_ptr<Texture>> material_textures;
};

#endif //TOPAZ_MODEL_HPP
