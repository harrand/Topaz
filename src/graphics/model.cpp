//
// Created by Harry on 03/01/2019.
//

#include "model.hpp"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

Model::Model(std::string filename): meshes(), material_textures()
{
    /// Assimp imports the scene...
    const aiScene* scene = aiImportFile(filename.c_str(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_TransformUVCoords | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_RemoveRedundantMaterials | aiProcess_FindInvalidData | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
    /// Get the directory containing the model file. This is used later.
    std::string directory = filename.substr(0, filename.find_last_of('/'));
    if(scene == nullptr)
    {
        tz::debug::print("Error: Model import failed:\n", aiGetErrorString(), "\n");
        return;
    }
    for(unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[i];
        // Can't emplace back because Mesh::Mesh(aiMesh*) is protected. Even though we're a friend, std::vector is not.
        Mesh tz_mesh{mesh};
        // So the next best thing is to move it in.
        this->meshes.push_back(std::move(tz_mesh));

        // Sort out materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<std::string> diffuse_maps, specular_maps;
        diffuse_maps = Model::load_material_texture_references(material, aiTextureType_DIFFUSE);
        // TODO: Implement SpecularMap before re-adding this feature to Model.
        //specular_maps = Model::load_material_texture_references(material, aiTextureType_SPECULAR);
        auto is_embedded = [&scene](const std::string& texture_reference)->bool
        {
            return tz::utility::string::begins_with(texture_reference, "*");
        };

        auto get_texture_id = [&scene](const std::string& texture_reference)->std::size_t
                {
                    std::string removed_asterisk = tz::utility::string::replace_all_char(texture_reference, '*', "");
                    std::size_t assimp_texture_id = tz::utility::generic::cast::from_string<std::size_t>(removed_asterisk);
                    return assimp_texture_id;
                };
        for(const std::string& reference : diffuse_maps)
        {
            if(!is_embedded(reference))
            {
                std::string appended_reference = directory + "/" + reference;
                tz::debug::print("Extracting model texture by file: \"", appended_reference, "\"\n");
                this->material_textures.insert({i, Texture{appended_reference}});
            }
            else
            {
                std::size_t mat_id = get_texture_id(reference);
                tz::debug::print("Extracting model texture by embedded material: ", mat_id, "\n");
                aiTexture *embedded_texture = scene->mTextures[mat_id];
                this->material_textures.insert({i, Texture{embedded_texture}});
            }
        }
        /*
        for(const std::string& reference : specular_maps)
        {
            aiTexture* embedded_texture = scene->mTextures[get_texture_id(reference)];
            this->material_textures.insert({i, Texture{embedded_texture}});
        }
         */
        /*
        for(const std::string& reference : diffuse_maps)
        {
            this->material_textures.insert({i, {directory + '/' + reference}});
        }
        for(const std::string& reference : specular_maps)
        {
            this->material_textures.insert({i, {directory + '/' + reference}});
        }
         */
        /*
        for(Texture& diffuse_map : diffuse_maps)
            this->material_textures.insert({i, std::move(diffuse_map)});
        for(Texture& specular_map : specular_maps)
            this->material_textures.insert({i, std::move(specular_map)});
            */
    }
    aiReleaseImport(scene);
}

std::size_t Model::get_number_of_meshes() const
{
    return this->meshes.size();
}

const Mesh* Model::get_mesh_by_id(std::size_t mesh_id) const
{
    try
    {
        return &this->meshes.at(mesh_id);
    }catch(const std::out_of_range&){return nullptr;}
}

std::size_t Model::get_number_of_textures() const
{
    return this->material_textures.size();
}

std::vector<std::reference_wrapper<const Texture>> Model::get_textures_in_mesh(std::size_t mesh_id) const
{
    std::vector<std::reference_wrapper<const Texture>> texture_refs;
    auto range = this->material_textures.equal_range(mesh_id);
    for(auto i = range.first; i != range.second; i++)
        texture_refs.push_back(std::cref(i->second));
    return texture_refs;
}

void Model::render(Shader& shader, GLenum mode) const
{
    bool patches = shader.has_tessellation_control_shader();
    for(unsigned int i = 0; i < this->meshes.size(); i++)
    {
        auto range = this->material_textures.equal_range(i);
        std::size_t texture_id = tz::graphics::texture_sampler_id;
        for(auto i = range.first; i != range.second; i++)
        {
            /* topaz shader attribs are *not* sequential. it's something a little like this:
             * 0 = texture_sampler
             * ...
             * 4 = extra_texture_sampler0
             * 5 = extra_texture_sampler1
             * and so on...
             */
            auto sampler_name = [](std::size_t id) -> std::string
            {
                if(id == tz::graphics::texture_sampler_id)
                    return "texture_sampler";
                else if(id >= tz::graphics::initial_extra_texture_sampler_id)
                {
                    std::size_t diff = id - tz::graphics::initial_extra_texture_sampler_id;
                    return std::string("extra_texture_sampler") + std::to_string(diff);
                }
                else
                {
                    tz::debug::print("Model has created an illegal sampler ID...");
                    return "";
                }
            };

            // bind each texture in the model
            i->second.bind(&shader, texture_id, sampler_name(texture_id));
            // ensure texture_id is updated properly, because as stated previous, topaz sampler ids are NOT sequential
            if(texture_id == tz::graphics::texture_sampler_id)
                texture_id = tz::graphics::initial_extra_texture_sampler_id;
            else
            {
                std::size_t diff = texture_id - tz::graphics::initial_extra_texture_sampler_id;
                std::string uniform_name = std::string("extra_texture") + std::to_string(diff) + "_exists";
                // TODO: Enable Specular Maps
                shader.set_uniform<bool>(uniform_name, true);
                texture_id++;
            }
        }
        this->meshes[i].render(patches, mode);
    }
}

std::vector<std::string> Model::load_material_texture_references(aiMaterial* material, aiTextureType type)
{
    std::vector<std::string> texture_references;
    for(unsigned int i = 0; i < material->GetTextureCount(type); i++)
    {
        aiString path;
        material->GetTexture(type, i, &path);
        texture_references.emplace_back(path.C_Str());
    }
    return texture_references;
}