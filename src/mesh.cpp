#include "mesh.hpp"

Vertex::Vertex(Vector3F position, Vector2F texcoord, Vector3F normal): position(std::move(position)), texcoord(std::move(texcoord)), normal(std::move(normal)){}

Mesh::Mesh(std::string filename): filename(std::move(filename)), model(OBJModel(this->filename).ToIndexedModel())
{
	this->initMesh();
}

Mesh::Mesh(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices)
{
	IndexedModel model;
	
	for(unsigned int i = 0; i < numVertices; i++)
	{
		model.positions.push_back(vertices[i].position);
		model.texcoords.push_back(vertices[i].texcoord);
		model.normals.push_back(vertices[i].normal);
	}
	
	for(unsigned int i = 0; i < numIndices; i++)
		model.indices.push_back(indices[i]);
	
	this->model = model;
	this->initMesh();
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &(this->vertexArrayObject));
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
	glBindVertexArray(this->vertexArrayObject);
	if(patches)
	{
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glDrawElements(GL_PATCHES, this->renderCount, GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawElements(GL_TRIANGLES, this->renderCount, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
}

//static
Mesh* Mesh::getFromLink(const std::string& meshLink, const std::vector<std::unique_ptr<Mesh>>& allMeshes)
{
	for(auto& mesh : allMeshes)
		if(mesh->getFileName() == meshLink)
			return mesh.get();
	return nullptr;
}

//private
void Mesh::initMesh()
{
	this->renderCount = this->model.indices.size();
	// Our vector class is not "c-enough" (contains stuff like protected variables which C structs don't support. Therefore we use VectorSXF instead which can work with OpenGL easily.
	// However our indices vector from model is fine as-is.
	std::vector<VectorS3F> positions;
	positions.reserve(this->model.positions.size());
    std::vector<VectorS2F> texcoords;
	texcoords.reserve(this->model.texcoords.size());
    std::vector<VectorS3F> normals;
	normals.reserve(this->model.normals.size());
	std::vector<VectorS3F> tangents;
	tangents.reserve(this->model.tangents.size());
	for(auto vec : this->model.positions)
		positions.push_back(vec.toRaw());
	for(auto vec : this->model.texcoords)
		texcoords.push_back(vec.toRaw());
	for(auto vec : this->model.normals)
		normals.push_back(vec.toRaw());
	for(auto vec : this->model.tangents)
		tangents.push_back(vec.toRaw());
	
	glGenVertexArrays(1, &(this->vertexArrayObject));
	glBindVertexArray(this->vertexArrayObject);
	
	glGenBuffers(static_cast<unsigned int>(BufferTypes::NUM_BUFFERS), this->vertexArrayBuffers);
	// 0 = Vertices, 1 = Texture Coordinates, 2 = Internal Normals, 3 = Indices, 4 = Tangents
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[static_cast<unsigned int>(BufferTypes::POSITION)]);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), positions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[static_cast<unsigned int>(BufferTypes::TEXCOORD)]);
	glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(texcoords[0]), texcoords.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[static_cast<unsigned int>(BufferTypes::NORMAL)]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vertexArrayBuffers[static_cast<unsigned int>(BufferTypes::INDEX)]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->model.indices.size() * sizeof(this->model.indices[0]), this->model.indices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_UNSIGNED_INT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[static_cast<unsigned int>(BufferTypes::TANGENT)]);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(tangents[0]), tangents.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}