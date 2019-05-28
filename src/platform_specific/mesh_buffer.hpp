//
// Created by Harry on 28/05/2019.
//

#ifndef TOPAZ_MESH_BUFFER_HPP
#define TOPAZ_MESH_BUFFER_HPP
#include "platform_specific/vertex_buffer.hpp"
#include "graphics/mesh.hpp"

namespace tz::platform
{
	class OGLMeshElement
	{
	public:
		OGLMeshElement(std::string filename);
		OGLMeshElement(const aiMesh* assimp_mesh);
		OGLMeshElement(const OGLMesh& mesh);
		friend class OGLMeshBuffer;
	private:
		OGLMeshElement();
		const OGLVertexArray* parent_array;
		/// List of all vertex-data.
		std::vector<Vertex> vertices;
		/// Element Array Buffer, essentially.
		std::vector<unsigned int> indices;
	};

	class OGLMeshBuffer
	{
	public:
		OGLMeshBuffer();
		template<typename... Args>
		OGLMeshElement& emplace_mesh(Args&&... args);
		void render() const;
		std::size_t get_size() const;
	private:
		void integrate_mesh(const OGLMeshElement& mesh) const;
		OGLVertexArray vao;
		OGLVertexBuffer& position_buffer;
		OGLVertexBuffer& texcoord_buffer;
		OGLVertexBuffer& normal_buffer;
		OGLVertexBuffer& tangent_buffer;
		OGLIndexBuffer& index_buffer;
		OGLVertexAttribute* position_attribute;
		OGLVertexAttribute* texcoord_attribute;
		OGLVertexAttribute* normal_attribute;
		OGLVertexAttribute* tangent_attribute;

		std::vector<std::unique_ptr<OGLMeshElement>> meshes;
		std::vector<GLsizei> index_counts;
	};
}

#include "platform_specific/mesh_buffer.inl"
#endif //TOPAZ_MESH_BUFFER_HPP
