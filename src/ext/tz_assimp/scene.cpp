#include "ext/tz_assimp/scene.hpp"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"

namespace tz::ext::assimp
{
	Scene::Scene(const std::string& filename): scene(aiImportFile(filename.c_str(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_TransformUVCoords | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_RemoveRedundantMaterials | aiProcess_FindInvalidData | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph)){}
	
	Scene::Scene(Scene&& move): scene(move.scene)
	{
		move.scene = nullptr;
	}

	Scene& Scene::operator=(Scene&& rhs)
	{
		std::swap(this->scene, rhs.scene);
		return *this;
	}

	Scene::~Scene()
	{
		if(this->scene != nullptr)
			aiReleaseImport(this->scene);
	}

	std::size_t Scene::size_meshes() const
	{
		return this->scene->mNumMeshes;
	}

	const aiMesh* Scene::get_mesh(std::size_t idx) const
	{
		return this->scene->mMeshes[idx];
	}

	const aiMaterial* Scene::get_material(std::size_t material_idx) const
	{
		return this->scene->mMaterials[material_idx];
	}
}