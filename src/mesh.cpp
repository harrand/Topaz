#include "mesh.hpp"

Vertex::Vertex(Vector3F position, Vector2F texcoord, Vector3F normal): position(position), texcoord(texcoord), normal(normal){}

Mesh::Mesh(std::string filename): filename(filename), model(OBJModel(filename).ToIndexedModel())
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

void Mesh::render() const
{
	glBindVertexArray(this->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, this->renderCount, GL_UNSIGNED_INT, 0);
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
	
	glGenVertexArrays(1, &(this->vertexArrayObject));
	glBindVertexArray(this->vertexArrayObject);
	
	glGenBuffers(static_cast<unsigned int>(BufferTypes::NUM_BUFFERS), this->vertexArrayBuffers);
	// 0 = Vertices, 1 = Texture Coordinates, 2 = Internal Normals, 3 = Tangents
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[static_cast<unsigned int>(BufferTypes::POSITION)]);
	glBufferData(GL_ARRAY_BUFFER, this->model.positions.size() * sizeof(this->model.positions[0]), &(this->model.positions[0]), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[static_cast<unsigned int>(BufferTypes::TEXCOORD)]);
	glBufferData(GL_ARRAY_BUFFER, this->model.texcoords.size() * sizeof(this->model.texcoords[0]), &(this->model.texcoords[0]), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[static_cast<unsigned int>(BufferTypes::NORMAL)]);
	glBufferData(GL_ARRAY_BUFFER, this->model.normals.size() * sizeof(this->model.normals[0]), &(this->model.normals[0]), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vertexArrayBuffers[static_cast<unsigned int>(BufferTypes::INDEX)]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->model.indices.size() * sizeof(this->model.indices[0]), &(this->model.indices[0]), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[static_cast<unsigned int>(BufferTypes::TANGENT)]);
	glBufferData(GL_ARRAY_BUFFER, this->model.tangents.size() * sizeof(this->model.tangents[0]), &(this->model.tangents[0]), GL_STATIC_DRAW);

	glBindVertexArray(0);
}