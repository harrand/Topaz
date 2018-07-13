#include "graphics/mesh.hpp"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <map>
#include <unordered_set>

Mesh::Mesh(std::string filename): filename(std::move(filename))
{
	const aiScene* scene = aiImportFile(this->filename.c_str(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_TransformUVCoords | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
	if(scene == nullptr)
    {
        std::cerr << "Error: Mesh import failed:\n" << aiGetErrorString() << "\n";
        return;
    }
    aiMesh* assimp_mesh = scene->mMeshes[0];
	/* things to assign:
	 * position (vec3)
	 * texcoord (vec2)
	 * normal (vec3)
	 * tangent (vec3)
	 * render_count (uint)
	 */
	// Right now, we don't know number of indices. We will find that out after we perform indexing.
	this->vertices.reserve(assimp_mesh->mNumVertices);
	for(std::size_t i = 0; i < assimp_mesh->mNumVertices; i++)
	{
		Vector3F position{assimp_mesh->mVertices[i].x, assimp_mesh->mVertices[i].y, assimp_mesh->mVertices[i].z};
        Vector2F texcoord = {};
        if(assimp_mesh->HasTextureCoords(0))
        {
            const aiVector3D* assimp_texcoord = &assimp_mesh->mTextureCoords[0][i];
            texcoord = {assimp_texcoord->x, assimp_texcoord->y};
        }
        Vector3F normal = {};
		if(assimp_mesh->HasNormals())
            normal = Vector3F{assimp_mesh->mNormals[i].x, assimp_mesh->mNormals[i].y, assimp_mesh->mNormals[i].z}.normalised();
        Vector3F tangent = {};
        if(assimp_mesh->HasTangentsAndBitangents())
		    tangent = Vector3F{assimp_mesh->mTangents[i].x, assimp_mesh->mTangents[i].y, assimp_mesh->mTangents[i].z}.normalised();
		this->vertices.emplace_back(position, texcoord, normal, tangent);
	}
    for(std::size_t i = 0; i < assimp_mesh->mNumFaces; i++)
    {
        const aiFace& face = assimp_mesh->mFaces[i];
        this->indices.push_back(face.mIndices[0]);
        this->indices.push_back(face.mIndices[1]);
        this->indices.push_back(face.mIndices[2]);
    }
	aiReleaseImport(scene);
    this->init_mesh();
}

Mesh::Mesh(const Vertex* vertices, std::size_t number_of_vertices, const unsigned int* indices, std::size_t number_of_indices)
{
	for(unsigned int i = 0; i < number_of_vertices; i++)
        this->vertices.push_back(vertices[i]);
	for(unsigned int i = 0; i < number_of_indices; i++)
		this->indices.push_back(indices[i]);
	this->init_mesh();
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices): Mesh(vertices.data(), vertices.size(), indices.data(), indices.size()){}

Mesh::~Mesh()
{
	for(GLuint& vbo_buffer : this->vbo_buffers)
		glDeleteBuffers(1, &vbo_buffer);
	glDeleteVertexArrays(1, &(this->vertex_array_object));
}

std::vector<Vector3F> Mesh::get_positions() const
{
	std::vector<Vector3F> positions;
	positions.reserve(this->vertices.size());
	for(const auto& vertex : this->vertices)
		positions.push_back(vertex.position);
	return positions;
}

std::vector<Vector2F> Mesh::get_texcoords() const
{
	std::vector<Vector2F> texture_coordinates;
	texture_coordinates.reserve(this->vertices.size());
	for(const auto& vertex : this->vertices)
		texture_coordinates.push_back(vertex.texture_coordinate);
	return texture_coordinates;
}

std::vector<Vector3F> Mesh::get_normals() const
{
	std::vector<Vector3F> normals;
	normals.reserve(this->vertices.size());
	for(const auto& vertex : this->vertices)
		normals.push_back(vertex.normal);
	return normals;
}

std::vector<Vector3F> Mesh::get_tangents() const
{
	std::vector<Vector3F> tangents;
	tangents.reserve(this->vertices.size());
	for(const auto& vertex : this->vertices)
		tangents.push_back(vertex.tangent);
	return tangents;
}

const std::vector<unsigned int>& Mesh::get_indices() const
{
	return this->indices;
}

std::string Mesh::get_file_name() const
{
	return this->filename;
}

void Mesh::render(bool patches, GLenum mode) const
{
	glBindVertexArray(this->vertex_array_object);
	if(patches)
	{
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glDrawElements(GL_PATCHES, this->indices.size(), GL_UNSIGNED_INT, NULL);
	}
	else
	{
		glDrawElements(mode, this->indices.size(), GL_UNSIGNED_INT, NULL);
	}
	glBindVertexArray(0);
}

bool Mesh::operator==(const Mesh& rhs) const
{
	return this->vertex_array_object == rhs.vertex_array_object;
}

//private
void Mesh::init_mesh()
{
	using tz::graphics::BufferTypes;
	// Our vector class is not "c-enough" (contains stuff like protected variables which C structs don't support. Therefore we use VectorSXF instead which can work with OpenGL easily.
	// However our indices vector from model is fine as-is.
	std::vector<Vector3POD> positions;
	positions.reserve(this->get_positions().size());
    std::vector<Vector2POD> texcoords;
	texcoords.reserve(this->get_texcoords().size());
    std::vector<Vector3POD> normals;
	normals.reserve(this->get_normals().size());
	std::vector<Vector3POD> tangents;
	tangents.reserve(this->get_tangents().size());
	for(auto vec : this->get_positions())
		positions.push_back(vec.to_raw());
	for(auto vec : this->get_texcoords())
		texcoords.push_back(vec.to_raw());
	for(auto vec : this->get_normals())
		normals.push_back(vec.to_raw());
	for(auto vec : this->get_tangents())
		tangents.push_back(vec.to_raw());

	glGenVertexArrays(1, &(this->vertex_array_object));
	glBindVertexArray(this->vertex_array_object);
	
	glGenBuffers(static_cast<unsigned int>(BufferTypes::NUM_BUFFERS), this->vbo_buffers.data());
	// 0 = Vertices, 1 = Texture Coordinates, 2 = Internal Normals, 3 = Indices, 4 = Tangents

	using namespace tz::utility; // tz::utility::generic::sizeof_element
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_buffers[static_cast<unsigned int>(BufferTypes::POSITION)]);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * generic::sizeof_element(positions), positions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_buffers[static_cast<unsigned int>(BufferTypes::TEXCOORD)]);
	glBufferData(GL_ARRAY_BUFFER, texcoords.size() * generic::sizeof_element(texcoords), texcoords.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_buffers[static_cast<unsigned int>(BufferTypes::NORMAL)]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * generic::sizeof_element(normals), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE,  3 * sizeof(float), NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_buffers[static_cast<unsigned int>(BufferTypes::TANGENT)]);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * generic::sizeof_element(tangents), tangents.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(float), NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_buffers[static_cast<unsigned int>(BufferTypes::INDEX)]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * generic::sizeof_element(this->indices), this->indices.data(), GL_STATIC_DRAW);
	
	glBindVertexArray(0);
}

InstancedMesh::InstancedMesh(std::string filename, std::vector<Vector3F> positions, std::vector<Vector3F> rotations, std::vector<Vector3F> scales): Mesh(filename), positions(positions), rotations(rotations), scales(scales), models({}), instance_quantity(std::max({this->positions.size(), this->rotations.size(), this->scales.size()}))
{
    // Build all of the model matrices.
    std::size_t number_of_matrices = positions.size();
    this->models.reserve(number_of_matrices);
    for(std::size_t i = 0; i < number_of_matrices; i++)
    {
        Vector3F position = this->positions[i];
        // Rotation and Scale aren't guaranteed to exist, so double check and default if not exist.
        Vector3F rotation, scale;
        try
        {
            rotation = this->rotations.at(i);
            scale = this->scales.at(i);
        }catch(const std::out_of_range& oor){rotation = {}; scale = {1, 1, 1};}
        this->models.push_back(tz::transform::model(position, rotation, scale));
    }

    // Foreach model matrix, populate each row so they can be individually sent to VRAM.
    std::vector<Vector4POD> xs, ys, zs, ws;
    for(Matrix4x4 model : this->models)
    {
        xs.push_back(model.x.to_raw());
        ys.push_back(model.y.to_raw());
        zs.push_back(model.z.to_raw());
        ws.push_back(model.w.to_raw());
    }

	using namespace tz::utility; // tz::utility::generic::sizeof_element
    // Populate Mesh::vertex_array_object with additional vbo buffers.
	glBindVertexArray(this->vertex_array_object);
    // Row X (attribute 4)
    glGenBuffers(1, &this->model_matrix_x_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->model_matrix_x_vbo);
    glBufferData(GL_ARRAY_BUFFER, xs.size() * generic::sizeof_element(xs), xs.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, this->model_matrix_x_vbo);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(4, 1);
    // Row Y (attribute 5)
    glGenBuffers(1, &this->model_matrix_y_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->model_matrix_y_vbo);
    glBufferData(GL_ARRAY_BUFFER, ys.size() * generic::sizeof_element(ys), ys.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, this->model_matrix_y_vbo);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glVertexAttribDivisor(5, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Row Z (attribute 6)
    glGenBuffers(1, &this->model_matrix_z_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->model_matrix_z_vbo);
    glBufferData(GL_ARRAY_BUFFER, zs.size() * generic::sizeof_element(zs), zs.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, this->model_matrix_z_vbo);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glVertexAttribDivisor(6, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Row W (attribute 7)
    glGenBuffers(1, &this->model_matrix_w_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->model_matrix_w_vbo);
    glBufferData(GL_ARRAY_BUFFER, ws.size() * generic::sizeof_element(ws), ws.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, this->model_matrix_w_vbo);
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glVertexAttribDivisor(7, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Finish working with our new vao.
	glBindVertexArray(0);
}

InstancedMesh::~InstancedMesh()
{
	glDeleteBuffers(1, &(this->model_matrix_x_vbo));
    glDeleteBuffers(1, &(this->model_matrix_y_vbo));
    glDeleteBuffers(1, &(this->model_matrix_z_vbo));
    glDeleteBuffers(1, &(this->model_matrix_w_vbo));
}

const std::vector<Vector3F>& InstancedMesh::get_instance_positions() const
{
	return this->positions;
}

const std::vector<Vector3F>& InstancedMesh::get_instance_rotations() const
{
	return this->rotations;
}

const std::vector<Vector3F>& InstancedMesh::get_instance_scales() const
{
	return this->scales;
}

const std::vector<Matrix4x4>& InstancedMesh::get_model_matrices() const
{
    return this->models;
}

std::size_t InstancedMesh::get_instance_quantity() const
{
	return this->instance_quantity;
}

void InstancedMesh::render(bool patches, GLenum mode) const
{
	glBindVertexArray(this->vertex_array_object);
	if(patches)
	{
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glDrawElementsInstanced(GL_PATCHES, this->indices.size(), GL_UNSIGNED_INT, nullptr, this->instance_quantity);
	}
	else
	{
		glDrawElementsInstanced(mode, this->indices.size(), GL_UNSIGNED_INT, nullptr, this->instance_quantity);
	}
	glBindVertexArray(0);
}

bool tz::graphics::is_instanced(const Mesh* mesh)
{
	return dynamic_cast<const InstancedMesh*>(mesh) != nullptr;
}

Mesh* tz::graphics::find_mesh(const std::string& mesh_link, const std::vector<std::unique_ptr<Mesh>>& all_meshes)
{
	for(auto& mesh : all_meshes)
		if(mesh->get_file_name() == mesh_link)
			return mesh.get();
	return nullptr;
}

Mesh tz::graphics::create_quad(float x, float y, float width, float height)
{
	std::array<Vertex, 4> vertices({Vertex({x - width, y - height, 0}, {}, {}), Vertex({x - width, y + height, 0}, {0, 1}, {}), Vertex({x + width, y + height, 0}, {1, 1}, {}), Vertex({x + width, y - height, 0}, {1, 0}, {})});
	std::array<unsigned int, 6> indices({0, 1, 2, 0, 2, 3});
	return {vertices.data(), vertices.size(), indices.data(), indices.size()};
}

Mesh tz::graphics::create_cube()
{
	std::vector<Vertex> vertices;
	vertices.emplace_back(Vector3F(-1, -1, -1), Vector2F(1, 0), Vector3F(0, 0, -1));
	vertices.emplace_back(Vector3F(-1, 1, -1), Vector2F(0, 0), Vector3F(0, 0, -1));
	vertices.emplace_back(Vector3F(1, 1, -1), Vector2F(0, 1), Vector3F(0, 0, -1));
	vertices.emplace_back(Vector3F(1, -1, -1), Vector2F(1, 1), Vector3F(0, 0, -1));

	vertices.emplace_back(Vector3F(-1, -1, 1), Vector2F(1, 0), Vector3F(0, 0, 1));
	vertices.emplace_back(Vector3F(-1, 1, 1), Vector2F(0, 0), Vector3F(0, 0, 1));
	vertices.emplace_back(Vector3F(1, 1, 1), Vector2F(0, 1), Vector3F(0, 0, 1));
	vertices.emplace_back(Vector3F(1, -1, 1), Vector2F(1, 1), Vector3F(0, 0, 1));

	vertices.emplace_back(Vector3F(-1, -1, -1), Vector2F(0, 1), Vector3F(0, -1, 0));
	vertices.emplace_back(Vector3F(-1, -1, 1), Vector2F(1, 1), Vector3F(0, -1, 0));
	vertices.emplace_back(Vector3F(1, -1, 1), Vector2F(1, 0), Vector3F(0, -1, 0));
	vertices.emplace_back(Vector3F(1, -1, -1), Vector2F(0, 0), Vector3F(0, -1, 0));

	vertices.emplace_back(Vector3F(-1, 1, -1), Vector2F(0, 1), Vector3F(0, 1, 0));
	vertices.emplace_back(Vector3F(-1, 1, 1), Vector2F(1, 1), Vector3F(0, 1, 0));
	vertices.emplace_back(Vector3F(1, 1, 1), Vector2F(1, 0), Vector3F(0, 1, 0));
	vertices.emplace_back(Vector3F(1, 1, -1), Vector2F(0, 0), Vector3F(0, 1, 0));

	vertices.emplace_back(Vector3F(-1, -1, -1), Vector2F(1, 1), Vector3F(-1, 0, 0));
	vertices.emplace_back(Vector3F(-1, -1, 1), Vector2F(1, 0), Vector3F(-1, 0, 0));
	vertices.emplace_back(Vector3F(-1, 1, 1), Vector2F(0, 0), Vector3F(-1, 0, 0));
	vertices.emplace_back(Vector3F(-1, 1, -1), Vector2F(0, 1), Vector3F(-1, 0, 0));

	vertices.emplace_back(Vector3F(1, -1, -1), Vector2F(1, 1), Vector3F(1, 0, 0));
	vertices.emplace_back(Vector3F(1, -1, 1), Vector2F(1, 0), Vector3F(1, 0, 0));
	vertices.emplace_back(Vector3F(1, 1, 1), Vector2F(0, 0), Vector3F(1, 0, 0));
	vertices.emplace_back(Vector3F(1, 1, -1), Vector2F(0, 1), Vector3F(1, 0, 0));
	return {vertices, {0, 1, 2, 0, 2, 3, 6, 5, 4, 7, 6, 4, 10, 9, 8, 11, 10, 8, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 22, 21, 20, 23, 22, 20}};
}