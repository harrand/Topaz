//
// Created by Harry on 28/05/2019.
//

#ifndef TOPAZ_MESH_BUFFER_HPP
#define TOPAZ_MESH_BUFFER_HPP
#include "platform/ogl/vertex_buffer.hpp"
#include "graphics/mesh.hpp"

namespace tz::gl
{
    /// Element of a MeshBuffer.
	class OGLMeshElement
	{
	public:
        /**
         * Construct a mesh element based upon a model filename.
         * @param filename - Path to the model file (e.g obj, fbx, etc...)
         */
		OGLMeshElement(std::string filename);
        /**
         * Construct a mesh element based upon an existing assimp mesh.
         * @param assimp_mesh - Assimp mesh to construct a copy of
         */
		OGLMeshElement(const aiMesh* assimp_mesh);
        /**
         * Deep copy constructor. Shall belong to the same VAO. Not useful unless you want to make vertex edits.
         * @param mesh - Mesh to construct the copy based off of
         */
		OGLMeshElement(const OGLMesh& mesh);
        /**
         * Convert to an OGLMesh to use in the old rendering system.
         * @return - OGLMesh containing identical vertices to this mesh element
         */
		explicit operator OGLMesh() const;
        bool operator==(const OGLMeshElement& rhs) const;
		friend class OGLMeshBuffer;
	private:
        /// Construct an empty mesh element. Does not currently belong to a VAO.
		OGLMeshElement();
        /// Parent VAO containing this mesh element.
		const OGLVertexArray* parent_array;
		/// List of all vertex-data.
		std::vector<Vertex> vertices;
		/// Element Array Buffer, essentially.
		std::vector<unsigned int> indices;
	};

    /**
     * Contains a list of mesh elements. This class should be used to render multiple meshes in a single draw call (MDI).
     * To render multiple Renderables in one draw call, consider using a RenderableBuffer<T> instead.
     */
	class OGLMeshBuffer
	{
	public:
        /**
         * Construct an empty mesh buffer.
         */
		OGLMeshBuffer();
        /**
         * Deep copy a mesh buffer. Shall contain deep copies of every element.
         * @param copy - Mesh buffer to make a copy from
         */
		OGLMeshBuffer(const OGLMeshBuffer& copy);
        /**
         * Move constructor for an existing mesh buffer. Far more performant than a deep copy.
         * @param move - Mesh buffer to move from
         */
		OGLMeshBuffer(OGLMeshBuffer&& move);
		OGLMeshBuffer& operator=(OGLMeshBuffer&& rhs);
        /**
         * Construct a mesh element in-place of this buffer and receive the resultant mesh element.
         * @tparam Args - Argument types to construct the mesh element
         * @param args - Argument values to construct the mesh element
         * @return - The constructed mesh element
         */
		template<typename... Args>
		OGLMeshElement& emplace_mesh(Args&&... args);
        /**
         * Render all meshes in the buffer in a single draw call using MDI.
         */
		void render() const;
        /**
         * Obtain the number of meshes inside of this buffer.
         * @return - Number of meshes in this buffer
         */
		std::size_t get_size() const;
        /**
         * Query as to whether this buffer contains a mesh with equal vertices to the parameter.
         * @param mesh - Mesh to check existence of
         * @return - True if the buffer contains a similar mesh, otherwise false
         */
        bool contains(const OGLMeshElement& mesh) const;
	private:
        /**
         * Add the vertex data of an inserted mesh into the underlying vertex buffers and attributes of this mesh buffer.
         * @param mesh - Mesh to integrate with the mesh buffer implementation
         */
		void integrate_mesh(const OGLMeshElement& mesh) const;
        /// VAO to store all mesh data.
		OGLVertexArray vao;
        /// Vertex buffer containing all position data of every mesh.
		OGLVertexBuffer* position_buffer;
        /// Vertex buffer containing all texture coordinates of every mesh.
		OGLVertexBuffer* texcoord_buffer;
        /// Vertex buffer containing all normals of every mesh.
		OGLVertexBuffer* normal_buffer;
        /// Vertex buffer containing all tangents of every mesh.
		OGLVertexBuffer* tangent_buffer;
        /// Index buffer containing all indices of each mesh.
		OGLIndexBuffer* index_buffer;
        /// Pointer to the position vertex attribute.
		OGLVertexAttribute* position_attribute;
        /// Pointer to the texture coordinate vertex attribute.
		OGLVertexAttribute* texcoord_attribute;
        /// Pointer to the normal vertex attribute.
		OGLVertexAttribute* normal_attribute;
        /// Pointer to the tangent vertex attribute.
		OGLVertexAttribute* tangent_attribute;

        /// Stores all mesh elements.
		std::vector<std::unique_ptr<OGLMeshElement>> meshes;
        /// Stores the number of indices of each mesh in the buffer. Guaranteed to be in the same order as the vector of mesh elements.
		std::vector<GLsizei> index_counts;
	};
}

#include "platform/ogl/mesh_buffer.inl"
#endif //TOPAZ_MESH_BUFFER_HPP
