#include "gl/mesh_loader.hpp"
#include "core/debug/assert.hpp"
#include "core/tz.hpp"
#include "core/resource_manager.hpp"

namespace tz::gl
{
	tz::gl::IndexedMesh load_mesh(const std::string& filename)
	{
		std::string full_path = tz::res().get_path() + filename;
		tz::ext::assimp::Scene scene{full_path};
		topaz_assert(scene.size_meshes() == 1, "tz::gl::load_mesh(", full_path, "): File must contain exactly one mesh to be loaded this way. This file contains ", scene.size_meshes(), " meshes...");
		const aiMesh* ass_mesh = scene.get_mesh();
		
		tz::gl::IndexedMesh mesh;
		#if TOPAZ_DEBUG
		std::vector<tz::Vec2> debug_texcoord_list;
		#endif

		// Chuck in all those vertices
		for(unsigned int i = 0; i < ass_mesh->mNumVertices; i++)
		{
			tz::gl::Vertex v;
			const aiVector3D& pos = ass_mesh->mVertices[i];
			v.position = {{pos.x, pos.y, pos.z}};

			if(ass_mesh->HasTextureCoords(0))
			{
				aiVector3D texcoord = ass_mesh->mTextureCoords[0][i];
				v.texcoord = {{texcoord.x, texcoord.y}};
				#if TOPAZ_DEBUG
				debug_texcoord_list.push_back(v.texcoord);
				#endif
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