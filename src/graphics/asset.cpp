#include "graphics/asset.hpp"

AssetBuffer::AssetBuffer(std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes, std::unordered_map<std::string, std::unique_ptr<Texture>> textures, std::unordered_map<std::string, std::unique_ptr<AnimatedTexture>> animated_textures, std::unordered_map<std::string, std::unique_ptr<Model>> models): meshes(std::move(meshes)), textures(std::move(textures)), animated_textures(std::move(animated_textures)), models(std::move(models)){}

bool AssetBuffer::sink_mesh(const std::string& asset_name, std::unique_ptr<Mesh> sunken_mesh)
{
    if(this->meshes.find(asset_name) != this->meshes.end())
        return false;
    this->meshes[asset_name] = std::move(sunken_mesh);
    return true;
}

bool AssetBuffer::sink_texture(const std::string& asset_name, std::unique_ptr<Texture> sunken_texture)
{
    if(this->textures.find(asset_name) != this->textures.end())
        return false;
    this->textures[asset_name] = std::move(sunken_texture);
    return true;
}

bool AssetBuffer::sink_animated_texture(const std::string &animation_name, std::unique_ptr<AnimatedTexture> sunken_animation)
{
    if(this->animated_textures.find(animation_name) != this->animated_textures.end())
        return false;
    this->animated_textures[animation_name] = std::move(sunken_animation);
    return true;
}

bool AssetBuffer::sink_model(const std::string& asset_name, std::unique_ptr<Model> sunken_model)
{
    if(this->models.find(asset_name) != this->models.end())
        return false;
    this->models[asset_name] = std::move(sunken_model);
    return true;
}

AnimatedTexture& AssetBuffer::emplace_animated_texture(const std::string& animation_name, PolyFrameTexture::FrameMap frames, unsigned int fps)
{
    auto pair_return = this->animated_textures.emplace(animation_name, std::make_unique<AnimatedTexture>(frames, fps));
    return *(*pair_return.first).second;
}

Mesh* AssetBuffer::find_mesh(const std::string& mesh_name)
{
    try
    {
        return this->meshes.at(mesh_name).get();
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

Texture* AssetBuffer::find_texture(const std::string& texture_name)
{
    try
    {
        return this->textures.at(texture_name).get();
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

AnimatedTexture* AssetBuffer::find_animated_texture(const std::string& animation_name)
{
    try
    {
        return this->animated_textures.at(animation_name).get();
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

Model* AssetBuffer::find_model(const std::string& model_name)
{
    try
    {
        return this->models.at(model_name).get();
    }catch(const std::out_of_range& exception)
    {
        return nullptr;
    }
}

std::unique_ptr<Mesh> AssetBuffer::take_mesh(const std::string& mesh_name)
{
    if(this->meshes.find(mesh_name) == this->meshes.end())
        return nullptr;
    std::unique_ptr<Mesh> mesh = std::move(this->meshes[mesh_name]);
    this->meshes.erase(mesh_name);
    return mesh;
}

std::unique_ptr<Texture> AssetBuffer::take_texture(const std::string& texture_name)
{
    if(this->textures.find(texture_name) == this->textures.end())
        return nullptr;
    std::unique_ptr<Texture> texture = std::move(this->textures[texture_name]);
    this->textures.erase(texture_name);
    return texture;
}

std::unique_ptr<AnimatedTexture> AssetBuffer::take_animated_texture(const std::string& animation_name)
{
    if(this->animated_textures.find(animation_name) == this->animated_textures.end())
        return nullptr;
    std::unique_ptr<AnimatedTexture> animation = std::move(this->animated_textures[animation_name]);
    this->animated_textures.erase(animation_name);
    return animation;
}

std::unique_ptr<Model> AssetBuffer::take_model(const std::string& model_name)
{
    if(this->models.find(model_name) == this->models.end())
        return nullptr;
    std::unique_ptr<Model> model = std::move(this->models[model_name]);
    this->models.erase(model_name);
    return model;
}

Asset::Asset(Mesh* mesh, Texture* texture, NormalMap* normal_map, ParallaxMap* parallax_map, DisplacementMap* displacement_map, SpecularMap* specular_map, EmissiveMap* emissive_map, Model* model): mesh(mesh), texture(texture), normal_map(normal_map), parallax_map(parallax_map), displacement_map(displacement_map), specular_map(specular_map), emissive_map(emissive_map), model(model){}

Asset::Asset(Mesh* mesh): Asset(mesh, nullptr){}
Asset::Asset(Texture* texture): Asset(nullptr, texture){}
Asset::Asset(NormalMap* normal_map): Asset(nullptr, nullptr, normal_map){}
Asset::Asset(ParallaxMap* parallax_map): Asset(nullptr, nullptr, nullptr, parallax_map){}
Asset::Asset(DisplacementMap* displacement_map): Asset(nullptr, nullptr, nullptr, nullptr, displacement_map){}
Asset::Asset(SpecularMap* specular_map): Asset(nullptr, nullptr, nullptr, nullptr, nullptr, specular_map){}
Asset::Asset(EmissiveMap* emissive_map): Asset(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, emissive_map){}
Asset::Asset(Model* model): Asset(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, model){}

bool Asset::valid_mesh() const
{
	return this->mesh != nullptr;
}

bool Asset::valid_texture() const
{
	return this->texture != nullptr;
}

bool Asset::valid_normal_map() const
{
	return this->normal_map != nullptr;
}

bool Asset::valid_parallax_map() const
{
	return this->parallax_map != nullptr;
}

bool Asset::valid_displacement_map() const
{
	return this->displacement_map != nullptr;
}

bool Asset::valid_specular_map() const
{
    return this->specular_map != nullptr;
}

bool Asset::valid_emissive_map() const
{
    return this->emissive_map != nullptr;
}

bool Asset::valid_model() const
{
    return this->model != nullptr;
}

bool Asset::is_renderable() const
{
    return this->valid_mesh() || this->valid_model();
}

bool Asset::operator==(const Asset &rhs) const
{
    return this->mesh == rhs.mesh && this->texture == rhs.texture && this->normal_map == rhs.normal_map && this->parallax_map == rhs.parallax_map && this->specular_map == rhs.specular_map && this->emissive_map == rhs.emissive_map && this->displacement_map == rhs.displacement_map;
}