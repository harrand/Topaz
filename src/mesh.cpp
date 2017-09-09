#include "mesh.hpp"

Mesh::Mesh(std::string filename): filename(std::move(filename)), model(OBJModel(this->filename).toIndexedModel())
{
	this->initMesh();
}

Mesh::Mesh(const Vertex* vertices, unsigned int number_of_vertices, const unsigned int* indices, unsigned int number_of_indices)
{
	IndexedModel model;
	
	for(unsigned int i = 0; i < number_of_vertices; i++)
	{
		model.positions.push_back(vertices[i].position);
		model.texcoords.push_back(vertices[i].texcoord);
		model.normals.push_back(vertices[i].normal);
	}
	
	for(unsigned int i = 0; i < number_of_indices; i++)
		model.indices.push_back(indices[i]);
	
	this->model = model;
	this->initMesh();
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices): Mesh(vertices.data(), vertices.size(), indices.data(), indices.size()){}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &(this->vertex_array_object));
}

IndexedModel Mesh::getIndexedModel() const
{
	return this->model;
}

const std::vector<Vector3F>& Mesh::getPositions() const
{
	return this->model.positions;
}

const std::vector<Vector2F>& Mesh::getTexcoords() const
{
	return this->model.texcoords;
}

const std::vector<Vector3F>& Mesh::getNormals() const
{
	return this->model.normals;
}

std::string Mesh::getFileName() const
{
	return this->filename;
}

void Mesh::render(bool patches) const
{
	glBindVertexArray(this->vertex_array_object);
	if(patches)
	{
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glDrawElements(GL_PATCHES, this->render_count, GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawElements(GL_TRIANGLES, this->render_count, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
}

//static
Mesh* Mesh::getFromLink(const std::string& mesh_link, const std::vector<std::unique_ptr<Mesh>>& all_meshes)
{
	for(auto& mesh : all_meshes)
		if(mesh->getFileName() == mesh_link)
			return mesh.get();
	return nullptr;
}

//private
void Mesh::initMesh()
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
		positions.push_back(vec.toRaw());
	for(auto vec : this->model.texcoords)
		texcoords.push_back(vec.toRaw());
	for(auto vec : this->model.normals)
		normals.push_back(vec.toRaw());
	for(auto vec : this->model.tangents)
		tangents.push_back(vec.toRaw());
	
	glGenVertexArrays(1, &(this->vertex_array_object));
	glBindVertexArray(this->vertex_array_object);
	
	glGenBuffers(static_cast<unsigned int>(BufferTypes::NUM_BUFFERS), this->vbo_buffers);
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

Mesh tz::graphics::createQuad()
{
	std::array<Vertex, 4> vertices({Vertex(Vector3F(), Vector2F(), Vector3F()), Vertex(Vector3F(0, 1, 0), Vector2F(0, 1), Vector3F()), Vertex(Vector3F(1, 1, 0), Vector2F(1, 1), Vector3F()), Vertex(Vector3F(0, 1, 0), Vector2F(1, 0), Vector3F())});
	std::array<unsigned int, 4> indices({0, 1, 2, 3});
	return Mesh(vertices.data(), vertices.size(), indices.data(), indices.size());
}