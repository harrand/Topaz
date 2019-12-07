//
// Created by Harry on 28/05/2019.
//

#include "mesh_buffer.hpp"
#include "assimp/scene.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"

namespace tz::gl
{
	OGLMeshElement::OGLMeshElement(std::string filename): OGLMeshElement()
	{
		const aiScene *scene = aiImportFile(filename.c_str(),aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_TransformUVCoords | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
		if (scene == nullptr)
		{
			tz::debug::print("OGLMesh::OGLMesh(filename, ...): Error: OGLMesh import failed:\n", aiGetErrorString(), "\n");
			return;
		}
		aiMesh *assimp_mesh = scene->mMeshes[0];
		OGLMeshElement to_assign{assimp_mesh};
		aiReleaseImport(scene);
		*this = std::move(to_assign);
	}

	OGLMeshElement::OGLMeshElement(const aiMesh *assimp_mesh): OGLMeshElement()
	{
		/* things to assign:
		 * position (vec3)
		 * texcoord (vec2)
		 * normal (vec3)
		 * tangent (vec3)
		 * render_count (uint)
		 */
		// Right now, we don't know number of indices. We will find that out after we perform indexing.
		this->vertices.reserve(assimp_mesh->mNumVertices);
		for (std::size_t i = 0; i < assimp_mesh->mNumVertices; i++)
		{
			Vector3F position{assimp_mesh->mVertices[i].x, assimp_mesh->mVertices[i].y, assimp_mesh->mVertices[i].z};
			Vector2F texcoord = {};
			if (assimp_mesh->HasTextureCoords(0))
			{
				const aiVector3D *assimp_texcoord = &assimp_mesh->mTextureCoords[0][i];
				texcoord = {assimp_texcoord->x, assimp_texcoord->y};
			}
			Vector3F normal = {};
			if (assimp_mesh->HasNormals())
				normal = Vector3F{assimp_mesh->mNormals[i].x, assimp_mesh->mNormals[i].y, assimp_mesh->mNormals[i].z}.normalised();
			Vector3F tangent = {};
			if (assimp_mesh->HasTangentsAndBitangents())
				tangent = Vector3F{assimp_mesh->mTangents[i].x, assimp_mesh->mTangents[i].y, assimp_mesh->mTangents[i].z}.normalised();
			this->vertices.emplace_back(position, texcoord, normal, tangent);
		}
		for (std::size_t i = 0; i < assimp_mesh->mNumFaces; i++)
		{
			const aiFace &face = assimp_mesh->mFaces[i];
			this->indices.push_back(face.mIndices[0]);
			this->indices.push_back(face.mIndices[1]);
			this->indices.push_back(face.mIndices[2]);
		}
	}

	OGLMeshElement::OGLMeshElement(const OGLMesh& mesh): OGLMeshElement()
	{
		std::size_t vertex_count = mesh.get_positions().size();
		this->vertices.reserve(vertex_count);
		for(std::size_t i = 0; i < vertex_count; i++)
			this->vertices.emplace_back(mesh.get_positions()[i], mesh.get_texcoords()[i], mesh.get_normals()[i], mesh.get_tangents()[i]);
		this->indices = mesh.get_indices();
	}

	OGLMeshElement::operator OGLMesh() const
	{
		return OGLMesh{this->vertices, this->indices};
	}

	bool OGLMeshElement::operator==(const OGLMeshElement& rhs) const
	{
		return this->vertices == rhs.vertices && this->indices == rhs.indices;
	}

	OGLMeshElement::OGLMeshElement(): parent_array(nullptr), vertices(), indices(){}

	OGLMeshBuffer::OGLMeshBuffer(): vao(), position_buffer(&vao.emplace_vertex_buffer()), texcoord_buffer(&vao.emplace_vertex_buffer()), normal_buffer(&vao.emplace_vertex_buffer()), tangent_buffer(&vao.emplace_vertex_buffer()), index_buffer(&vao.emplace_index_buffer()), position_attribute(nullptr), texcoord_attribute(nullptr), normal_attribute(nullptr), tangent_attribute(nullptr), meshes()
	{
		this->position_buffer->bind();
		this->position_attribute = &this->vao.emplace_vertex_attribute(0);
		this->position_attribute->define<float>(3, GL_FALSE, 3 * sizeof(float));
		this->texcoord_buffer->bind();
		this->texcoord_attribute = &this->vao.emplace_vertex_attribute(1);
		this->texcoord_attribute->define<float>(2, GL_FALSE, 2 * sizeof(float));
		this->normal_buffer->bind();
		this->normal_attribute = &this->vao.emplace_vertex_attribute(2);
		this->normal_attribute->define<float>(3, GL_TRUE, 3 * sizeof(float));
		this->tangent_buffer->bind();
		this->tangent_attribute = &this->vao.emplace_vertex_attribute(3);
		this->tangent_attribute->define<float>(3, GL_TRUE, 3 * sizeof(float));
	}

	OGLMeshBuffer::OGLMeshBuffer(const OGLMeshBuffer& copy): OGLMeshBuffer()
	{
		for(const auto& mesh_element_ptr : copy.meshes)
			this->emplace_mesh(*mesh_element_ptr);
	}

	OGLMeshBuffer::OGLMeshBuffer(OGLMeshBuffer&& move): vao(std::move(move.vao)), position_buffer(vao.vertex_buffers[0].get()), texcoord_buffer(vao.vertex_buffers[1].get()), normal_buffer(vao.vertex_buffers[2].get()), tangent_buffer(vao.vertex_buffers[3].get()), index_buffer(vao.index_buffer.get()), position_attribute(vao.vertex_attributes[0].get()), texcoord_attribute(vao.vertex_attributes[1].get()), normal_attribute(vao.vertex_attributes[2].get()), tangent_attribute(vao.vertex_attributes[3].get()), meshes(std::move(move.meshes)), index_counts(std::move(move.index_counts))
	{
		for(auto& mesh_ptr : this->meshes)
			mesh_ptr->parent_array = &this->vao;
	}

	OGLMeshBuffer& OGLMeshBuffer::operator=(OGLMeshBuffer&& rhs)
	{
		this->vao = std::move(rhs.vao);
		this->position_buffer = vao.vertex_buffers[0].get();
		this->texcoord_buffer = vao.vertex_buffers[1].get();
		this->normal_buffer = vao.vertex_buffers[2].get();
		this->tangent_buffer = vao.vertex_buffers[3].get();
		this->index_buffer = vao.index_buffer.get();
		this->position_attribute = vao.vertex_attributes[0].get();
		this->texcoord_attribute = vao.vertex_attributes[1].get();
		this->normal_attribute = vao.vertex_attributes[2].get();
		this->tangent_attribute = vao.vertex_attributes[3].get();
		this->meshes = std::move(rhs.meshes);
		this->index_counts = std::move(rhs.index_counts);
		return *this;
	}

	void OGLMeshBuffer::integrate_mesh(const OGLMeshElement& mesh) const
	{
		std::size_t vertex_count = mesh.vertices.size();
		std::vector<std::array<float, 3>> positions;
		positions.reserve(vertex_count);
		std::vector<std::array<float, 2>> texcoords;
		texcoords.reserve(vertex_count);
		std::vector<std::array<float, 3>> normals;
		normals.reserve(vertex_count);
		std::vector<std::array<float, 3>> tangents;
		tangents.reserve(vertex_count);
		for(const auto& vertex : mesh.vertices)
		{
			positions.push_back(vertex.position.data());
			texcoords.push_back(vertex.texture_coordinate.data());
			normals.push_back(vertex.normal.data());
			tangents.push_back(vertex.tangent.data());
		}
		
		// Upload to necessary buffers.
		// First we retrieve any existing buffer information.
		std::optional<decltype(positions)> current_positions = this->position_buffer->query_all_data<std::vector, decltype(positions)::value_type>();
		std::optional<decltype(texcoords)> current_texcoords = this->texcoord_buffer->query_all_data<std::vector, decltype(texcoords)::value_type>();
		std::optional<decltype(normals)> current_normals = this->normal_buffer->query_all_data<std::vector, decltype(normals)::value_type>();
		std::optional<decltype(tangents)> current_tangents = this->tangent_buffer->query_all_data<std::vector, decltype(tangents)::value_type>();
		if(current_positions.has_value())
		{
			for(auto i = current_positions.value().rbegin(); i != current_positions.value().rend(); i++)
				positions.insert(positions.begin(), *i);
		}
		if(current_texcoords.has_value())
		{
			for(auto i = current_texcoords.value().rbegin(); i != current_texcoords.value().rend(); i++)
				texcoords.insert(texcoords.begin(), *i);
		}
		if(current_normals.has_value())
		{
			for(auto i = current_normals.value().rbegin(); i != current_normals.value().rend(); i++)
				normals.insert(normals.begin(), *i);
		}
		if(current_tangents.has_value())
		{
			for(auto i = current_tangents.value().rbegin(); i != current_tangents.value().rend(); i++)
				tangents.insert(tangents.begin(), *i);
		}
		auto default_usage = OGLBufferUsage{OGLBufferFrequency::STATIC, OGLBufferNature::DRAW};
		// Then re-upload the whole thing.
		this->position_buffer->insert(positions, default_usage);
		this->texcoord_buffer->insert(texcoords, default_usage);
		this->normal_buffer->insert(normals, default_usage);
		this->tangent_buffer->insert(tangents, default_usage);

		std::vector<unsigned int> indices = mesh.indices;
		// Now append indices.
		std::optional<std::vector<unsigned int>> current_indices = this->index_buffer->query_all_data<std::vector, unsigned int>();
		if(current_indices.has_value())
		{
			auto max_current_index = *std::max_element(current_indices.value().begin(), current_indices.value().end());
			for(auto& index : indices)
				index += max_current_index;
			for(auto i = current_indices.value().rbegin(); i != current_indices.value().rend(); i++)
				indices.insert(indices.begin(), *i);
		}
		this->index_buffer->insert(indices, default_usage);
	}

	void OGLMeshBuffer::render() const
	{
		this->position_attribute->enable();
		this->texcoord_attribute->enable();
		this->normal_attribute->enable();
		this->tangent_attribute->enable();
		glMultiDrawElements(GL_TRIANGLES, this->index_counts.data(), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(this->index_counts.size()));

		// we run into the problem: how are we going to upload our uniforms? and how do we know which uniform belongs to who?
		// also this code is untested and might not even work.
	}

	std::size_t OGLMeshBuffer::get_size() const
	{
		return this->index_counts.size();
	}

	bool OGLMeshBuffer::contains(const OGLMeshElement& mesh) const
	{
		for(const auto& mesh_ptr : this->meshes)
			if(*mesh_ptr == mesh)
				return true;
		return false;
	}
}