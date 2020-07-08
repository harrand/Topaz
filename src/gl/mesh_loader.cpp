#include "gl/mesh_loader.hpp"
#include "core/debug/assert.hpp"
#include "core/core.hpp"
#include "core/resource_manager.hpp"

namespace tz::gl
{
	tz::gl::IndexedMesh load_mesh(const std::string& filename)
	{
		std::string full_path = tz::core::res().get_path() + filename;
		tz::ext::assimp::Scene scene{full_path};
		topaz_assert(scene.size_meshes() == 1, "tz::gl::load_mesh(", full_path, "): File must contain exactly one mesh to be loaded this way. This file contains ", scene.size_meshes(), " meshes...");
		const aiMesh* ass_mesh = scene.get_mesh();
		
		tz::gl::IndexedMesh mesh;

		// Chuck in all those vertices
		for(unsigned int i = 0; i < ass_mesh->mNumVertices; i++)
		{
			tz::gl::Vertex v;
			const aiVector3D& pos = ass_mesh->mVertices[i];
			v.position = {{pos.x, pos.y, pos.z}};

			if(ass_mesh->HasTextureCoords(i))
			{
				const aiVector3D* texcoord = ass_mesh->mTextureCoords[i];
				v.texcoord = {{texcoord->x, texcoord->y}};
			}

			const aiVector3D& norm = ass_mesh->mNormals[i];
			v.normal = {{norm.x, norm.y, norm.z}};

			const aiVector3D& tang = ass_mesh->mTangents[i];
			v.tangent = {{tang.x, tang.y, tang.z}};

			const aiVector3D& bitang = ass_mesh->mBitangents[i];
			v.bitangent = {{bitang.x, bitang.y, bitang.z}};

			mesh.vertices.push_back(std::move(v));
		}

		for(unsigned int i = 0; i < ass_mesh->mNumFaces; i++)
		{
			const aiFace& face = ass_mesh->mFaces[i];
			mesh.indices.push_back(face.mIndices[0]);
			mesh.indices.push_back(face.mIndices[1]);
			mesh.indices.push_back(face.mIndices[2]);
		}

		return mesh;
	}
}