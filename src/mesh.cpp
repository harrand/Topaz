#include "mesh.hpp"

Mesh::Mesh(std::string filename): filename(std::move(filename)), model(tz::graphics::model::OBJModel(this->filename).to_indexed_model())
{
	this->init_mesh();
}

Mesh::Mesh(const Vertex* vertices, std::size_t number_of_vertices, const unsigned int* indices, std::size_t number_of_indices)
{
	tz::graphics::model::IndexedModel model;
	
	for(unsigned int i = 0; i < number_of_vertices; i++)
	{
		model.positions.push_back(vertices[i].get_position());
		model.texcoords.push_back(vertices[i].get_texture_coordinate());
		model.normals.push_back(vertices[i].get_normal());
	}
	
	for(unsigned int i = 0; i < number_of_indices; i++)
		model.indices.push_back(indices[i]);
	
	this->model = model;
	this->init_mesh();
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices): Mesh(vertices.data(), vertices.size(), indices.data(), indices.size()){}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &(this->vertex_array_object));
}

tz::graphics::model::IndexedModel Mesh::get_indexed_model() const
{
	return this->model;
}

const std::vector<Vector3F>& Mesh::get_positions() const
{
	return this->model.positions;
}

const std::vector<Vector2F>& Mesh::get_texcoords() const
{
	return this->model.texcoords;
}

const std::vector<Vector3F>& Mesh::get_normals() const
{
	return this->model.normals;
}

const std::vector<Vector3F>& Mesh::get_tangents() const
{
	return this->model.tangents;
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
		glDrawElements(GL_PATCHES, this->render_count, GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawElements(mode, this->render_count, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
}

//private
void Mesh::init_mesh()
{
	this->render_count = this->model.indices.size();
	// Our vector class is not "c-enough" (contains stuff like protected variables which C structs don't support. Therefore we use VectorSXF instead which can work with OpenGL easily.
	// However our indices vector from model is fine as-is.
	std::vector<Vector3POD> positions;
	positions.reserve(this->model.positions.size());
    std::vector<Vector2POD> texcoords;
	texcoords.reserve(this->model.texcoords.size());
    std::vector<Vector3POD> normals;
	normals.reserve(this->model.normals.size());
	std::vector<Vector3POD> tangents;
	tangents.reserve(this->model.tangents.size());
	for(auto vec : this->model.positions)
		positions.push_back(vec.to_raw());
	for(auto vec : this->model.texcoords)
		texcoords.push_back(vec.to_raw());
	for(auto vec : this->model.normals)
		normals.push_back(vec.to_raw());
	for(auto vec : this->model.tangents)
		tangents.push_back(vec.to_raw());
	
	glGenVertexArrays(1, &(this->vertex_array_object));
	glBindVertexArray(this->vertex_array_object);
	
	glGenBuffers(static_cast<unsigned int>(BufferTypes::NUM_BUFFERS), this->vbo_buffers.data());
	// 0 = Vertices, 1 = Texture Coordinates, 2 = Internal Normals, 3 = Indices, 4 = Tangents
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_buffers[static_cast<unsigned int>(BufferTypes::POSITION)]);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), positions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_buffers[static_cast<unsigned int>(BufferTypes::TEXCOORD)]);
	glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(texcoords[0]), texcoords.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_buffers[static_cast<unsigned int>(BufferTypes::NORMAL)]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_buffers[static_cast<unsigned int>(BufferTypes::INDEX)]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->model.indices.size() * sizeof(this->model.indices[0]), this->model.indices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_UNSIGNED_INT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo_buffers[static_cast<unsigned int>(BufferTypes::TANGENT)]);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(tangents[0]), tangents.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

InstancedMesh::InstancedMesh(Mesh* mesh, std::vector<Vector3F> positions, std::vector<Vector3F> rotations, std::vector<Vector3F> scales): mesh(mesh), positions(positions), rotations(rotations), scales(scales)
{
	std::vector<Vector3POD> positions_pod, rotations_pod, scales_pod;
	for(Vector3F position : this->positions)
		positions_pod.push_back(position.to_raw());
	for(Vector3F rotation : this->rotations)
		rotations_pod.push_back(rotation.to_raw());
	for(Vector3F scale : this->scales)
		scales_pod.push_back(scale.to_raw());
	
	glGenBuffers(1, &instance_positions_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, instance_positions_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3POD) * this->positions.size(), positions_pod.data(), GL_STATIC_DRAW);
	
	glGenBuffers(1, &instance_rotations_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, instance_rotations_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3POD) * this->rotations.size(), rotations_pod.data(), GL_STATIC_DRAW);
	
	glGenBuffers(1, &instance_scales_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, instance_scales_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3POD) * this->scales.size(), scales_pod.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glEnableVertexAttribArray(5);
	glBindBuffer(GL_ARRAY_BUFFER, instance_positions_vbo);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glVertexAttribDivisor(5, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glEnableVertexAttribArray(6);
	glBindBuffer(GL_ARRAY_BUFFER, instance_rotations_vbo);
	glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glVertexAttribDivisor(6, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glEnableVertexAttribArray(7);
	glBindBuffer(GL_ARRAY_BUFFER, instance_scales_vbo);
	glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glVertexAttribDivisor(7, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InstancedMesh::render(bool patches, GLenum mode)
{
	glBindVertexArray(this->vertex_array_object);
	if(patches)
	{
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glDrawElementsInstanced(GL_PATCHES, this->render_count, GL_UNSIGNED_INT, 0, this->positions.size());
	}
	else
	{
		glDrawElementsInstanced(mode, this->render_count, GL_UNSIGNED_INT, 0, this->positions.size());
	}
	glBindVertexArray(0);
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
	std::array<Vertex, 4> vertices({Vertex(Vector3F(x + -1 * width, y + -1 * height, 0), Vector2F(), Vector3F()), Vertex(Vector3F(x + -1 * width, y + 1 * height, 0), Vector2F(0, 1), Vector3F()), Vertex(Vector3F(x + 1 * width, y + 1 * height, 0), Vector2F(1, 1), Vector3F()), Vertex(Vector3F(x + 1 * width, y + -1 * height, 0), Vector2F(1, 0), Vector3F())});
	std::array<unsigned int, 6> indices({0, 1, 2, 0, 2, 3});
	return {vertices.data(), vertices.size(), indices.data(), indices.size()};
}