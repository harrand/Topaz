#include "graphics.hpp"
#include <map>

namespace tz::graphics::model
{
static bool compareOBJIndexPointer(const OBJIndex* a, const OBJIndex* b);
static inline unsigned int findNextChar(unsigned int start, const char* str, unsigned int length, char token);
static inline unsigned int parseOBJIndexValue(const std::string& token, unsigned int start, unsigned int end);
static inline float parseOBJFloatValue(const std::string& token, unsigned int start, unsigned int end);
}

Vertex::Vertex(Vector3F position, Vector2F texcoord, Vector3F normal): position(std::move(position)), texcoord(std::move(texcoord)), normal(std::move(normal)){}

const Vector3F& Vertex::getPosition() const
{
	return this->position;
}

const Vector2F& Vertex::getTextureCoordinate() const
{
	return this->texcoord;
}

const Vector3F& Vertex::getNormal() const
{
	return this->normal;
}

Vector3F& Vertex::getPositionR()
{
	return this->position;
}

Vector2F& Vertex::getTextureCoordinateR()
{
	return this->texcoord;
}

Vector3F& Vertex::getNormalR()
{
	return this->normal;
}

namespace tz::graphics::model
{
	OBJModel::OBJModel(const std::string& file_name): has_uvs(false), has_normals(false)
	{
		std::ifstream file;
		file.open(file_name.c_str());
	
		std::string line;
		if(file.is_open())
		{
			while(file.good())
			{
				std::getline(file, line);
			
				std::size_t line_length = line.length();
				
				if(line_length < 2)
					continue;
				
				const char* line_c_str = line.c_str();
				
				switch(line_c_str[0])
				{
					case 'v':
						if(line_c_str[1] == 't')
							this->uvs.push_back(parseOBJVector2F(line));
						else if(line_c_str[1] == 'n')
							this->normals.push_back(parseOBJVector3F(line));
						else if(line_c_str[1] == ' ' || line_c_str[1] == '\t')
							this->vertices.push_back(parseOBJVector3F(line));
					break;
					case 'f':
						createOBJFace(line);
					break;
					default: break;
				};
			}
		}
		else
		{
			tz::util::log::error("Unable to load mesh: ", file_name);
		}
	}
	
	void IndexedModel::calculateNormals()
	{
		for(std::size_t i = 0; i < indices.size(); i += 3)
		{
			int i0 = indices[i];
			int i1 = indices[i + 1];
			int i2 = indices[i + 2];
	
			Vector3F v1 = (positions[i1] - positions[i0]);
			Vector3F v2 = (positions[i2] - positions[i0]);
			
			Vector3F normal = (v1.cross(v2)).normalised();//glm::normalize(glm::cross(v1, v2));
				
			normals[i0] = (normals[i0] + normal);
			normals[i1] = (normals[i1] + normal);
			normals[i2] = (normals[i2] + normal);
		}
		
		for(std::size_t i = 0; i < positions.size(); i++)
			normals[i] = normals[i].normalised();
	}
	
	void IndexedModel::calculateTangents()
	{
		for(std::size_t i = 0; i < indices.size(); i += 3)
		{
			int i0 = indices[i];
			int i1 = indices[i + 1];
			int i2 = indices[i + 2];
			Vector3F edge1 = (positions[i1] - positions[i0]);
			Vector3F edge2 = (positions[i2] - positions[i0]);
			float deltaU1 = texcoords.at(i1).getX() - texcoords.at(i0).getX();
			float deltaU2 = texcoords.at(i2).getX() - texcoords.at(i0).getX();
			float deltaV1 = texcoords.at(i1).getY() - texcoords.at(i0).getY();
			float deltaV2 = texcoords.at(i2).getY() - texcoords.at(i0).getY();
			float f = 1.0f/(deltaU1 * deltaV2 - deltaU2 * deltaV1);
			Vector3F tangent;
			tangent.getXR() = f * (deltaV2 * edge1.getX() - deltaV1 * edge2.getX());
			tangent.getYR() = f * (deltaV2 * edge1.getY() - deltaV1 * edge2.getY());
			tangent.getZR() = f * (deltaV2 * edge1.getZ() - deltaV1 * edge2.getZ());
			tangents.at(i0) += tangent;
			tangents.at(i1) += tangent;
			tangents.at(i2) += tangent;
		}
		for(std::size_t i = 0; i < tangents.size(); i++)
			tangents[i] = tangents[i].normalised();
	}
	
	IndexedModel OBJModel::toIndexedModel()
	{
		IndexedModel result;
		IndexedModel normal_model;
		std::size_t number_of_indices = obj_indices.size();
		normal_model.tangents.resize(number_of_indices);
		result.tangents.resize(number_of_indices);
		std::vector<OBJIndex*> index_lookup;
		for(std::size_t i = 0; i < number_of_indices; i++)
			index_lookup.push_back(&obj_indices[i]);
		std::sort(index_lookup.begin(), index_lookup.end(), compareOBJIndexPointer);
		std::map<OBJIndex, unsigned int> normal_model_index_map;
		std::map<unsigned int, unsigned int> index_map;
		for(std::size_t i = 0; i < number_of_indices; i++)
		{
			OBJIndex* current_index = &obj_indices[i];
			Vector3F current_position = vertices[current_index->vertex_index];
			Vector2F current_texture_coordinate;
			Vector3F current_normal;
			if(has_uvs)
				current_texture_coordinate = uvs[current_index->uv_index];
			else
				current_texture_coordinate = Vector2F(0,0);
			if(has_normals)
				current_normal = normals[current_index->normal_index];
			else
				current_normal = Vector3F(0,0,0);
			unsigned int normal_model_index;
			unsigned int result_model_index;
			std::map<OBJIndex, unsigned int>::iterator it = normal_model_index_map.find(*current_index);
			if(it == normal_model_index_map.end())
			{
				normal_model_index = normal_model.positions.size();
			
				normal_model_index_map.emplace(*current_index, normal_model_index);
				normal_model.positions.push_back(current_position);
				normal_model.texcoords.push_back(current_texture_coordinate);
				normal_model.normals.push_back(current_normal);
			}
			else
				normal_model_index = it->second;
			unsigned int previous_vertex_location = findLastVertexIndex(index_lookup, current_index, result);
			if(previous_vertex_location == static_cast<unsigned int>(-1))
			{
				result_model_index = result.positions.size();
			
				result.positions.push_back(current_position);
				result.texcoords.push_back(current_texture_coordinate);
				result.normals.push_back(current_normal);
			}
			else
				result_model_index = previous_vertex_location;
			normal_model.indices.push_back(normal_model_index);
			result.indices.push_back(result_model_index);
			index_map.emplace(result_model_index, normal_model_index);
		}
		if(!has_normals)
		{
			normal_model.calculateNormals();
			
			for(std::size_t i = 0; i < result.positions.size(); i++)
				result.normals[i] = normal_model.normals[index_map[i]];
		}
		normal_model.calculateTangents();
		for(std::size_t i = 0; i < result.tangents.size(); i++)
			result.tangents.at(i) = normal_model.tangents[index_map[i]];
		return result;
	}
	
	unsigned int OBJModel::findLastVertexIndex(const std::vector<OBJIndex*>& index_lookup, const OBJIndex* current_index, const IndexedModel& result)
	{
		std::size_t start = 0;
		std::size_t end = index_lookup.size();
		std::size_t current = (end - start) / 2 + start;
		std::size_t previous = start;
		
		while(current != previous)
		{
			OBJIndex* test_index = index_lookup[current];
			
			if(test_index->vertex_index == current_index->vertex_index)
			{
				std::size_t count_start = current;
			
				for(std::size_t i = 0; i < current; i++)
				{
					OBJIndex* possible_index = index_lookup[current - i];
					
					if(possible_index == current_index)
						continue;
						
					if(possible_index->vertex_index != current_index->vertex_index)
						break;
						
					count_start--;
				}
				
				for(std::size_t i = count_start; i < index_lookup.size() - count_start; i++)
				{
					OBJIndex* possible_index = index_lookup[current + i];
					
					if(possible_index == current_index)
						continue;
						
					if(possible_index->vertex_index != current_index->vertex_index)
						break;
					else if((!has_uvs || possible_index->uv_index == current_index->uv_index) 
						&& (!has_normals || possible_index->normal_index == current_index->normal_index))
					{
						Vector3F current_position = vertices[current_index->vertex_index];
						Vector2F current_texture_coordinate;
						Vector3F current_normal;
						
						if(has_uvs)
							current_texture_coordinate = uvs[current_index->uv_index];
						else
							current_texture_coordinate = Vector2F(0,0);
							
						if(has_normals)
							current_normal = normals[current_index->normal_index];
						else
							current_normal = Vector3F(0,0,0);
						
						for(std::size_t j = 0; j < result.positions.size(); j++)
						{
							if(current_position == result.positions[j] 
								&& ((!has_uvs || current_texture_coordinate == result.texcoords[j])
								&& (!has_normals || current_normal == result.normals[j])))
							{
								return j;
							}
						}
					}
				}
			
				return -1;
			}
			else
			{
				if(test_index->vertex_index < current_index->vertex_index)
					start = current;
				else
					end = current;
			}
		
			previous = current;
			current = (end - start) / 2 + start;
		}
		
		return -1;
	}
	
	void OBJModel::createOBJFace(const std::string& line)
	{
		std::vector<std::string> tokens = tz::util::string::splitString(line, ' ');
	
		this->obj_indices.push_back(parseOBJIndex(tokens[1], &this->has_uvs, &this->has_normals));
		this->obj_indices.push_back(parseOBJIndex(tokens[2], &this->has_uvs, &this->has_normals));
		this->obj_indices.push_back(parseOBJIndex(tokens[3], &this->has_uvs, &this->has_normals));
	
		if(tokens.size() > 4)
		{
			this->obj_indices.push_back(parseOBJIndex(tokens[1], &this->has_uvs, &this->has_normals));
			this->obj_indices.push_back(parseOBJIndex(tokens[3], &this->has_uvs, &this->has_normals));
			this->obj_indices.push_back(parseOBJIndex(tokens[4], &this->has_uvs, &this->has_normals));
		}
	}
	
	OBJIndex OBJModel::parseOBJIndex(const std::string& token, bool* has_uvs, bool* has_normals)
	{
		std::size_t token_length = token.length();
		const char* token_c_string = token.c_str();
		
		unsigned int vertex_index_start = 0;
		unsigned int vertex_index_end = findNextChar(vertex_index_start, token_c_string, token_length, '/');
		
		OBJIndex result;
		result.vertex_index = parseOBJIndexValue(token, vertex_index_start, vertex_index_end);
		result.uv_index = 0;
		result.normal_index = 0;
		
		if(vertex_index_end >= token_length)
			return result;
		
		vertex_index_start = vertex_index_end + 1;
		vertex_index_end = findNextChar(vertex_index_start, token_c_string, token_length, '/');
		
		result.uv_index = parseOBJIndexValue(token, vertex_index_start, vertex_index_end);
		*has_uvs = true;
		
		if(vertex_index_end >= token_length)
			return result;
		
		vertex_index_start = vertex_index_end + 1;
		vertex_index_end = findNextChar(vertex_index_start, token_c_string, token_length, '/');
		
		result.normal_index = parseOBJIndexValue(token, vertex_index_start, vertex_index_end);
		*has_normals = true;
		
		return result;
	}
	
	Vector3F OBJModel::parseOBJVector3F(const std::string& line) 
	{
		std::size_t token_length = line.length();
		const char* token_c_string = line.c_str();
		
		std::size_t vertex_index_start = 2;
		
		while(vertex_index_start < token_length)
		{
			if(token_c_string[vertex_index_start] != ' ')
				break;
			vertex_index_start++;
		}
		
		unsigned int vertex_index_end = findNextChar(vertex_index_start, token_c_string, token_length, ' ');
		
		float x = parseOBJFloatValue(line, vertex_index_start, vertex_index_end);
		
		vertex_index_start = vertex_index_end + 1;
		vertex_index_end = findNextChar(vertex_index_start, token_c_string, token_length, ' ');
		
		float y = parseOBJFloatValue(line, vertex_index_start, vertex_index_end);
		
		vertex_index_start = vertex_index_end + 1;
		vertex_index_end = findNextChar(vertex_index_start, token_c_string, token_length, ' ');
		
		float z = parseOBJFloatValue(line, vertex_index_start, vertex_index_end);
		
		return {x,y,z};
	}
	
	Vector2F OBJModel::parseOBJVector2F(const std::string& line)
	{
		std::size_t token_length = line.length();
		const char* token_c_string = line.c_str();
		
		std::size_t vertex_index_start = 3;
		
		while(vertex_index_start < token_length)
		{
			if(token_c_string[vertex_index_start] != ' ')
				break;
			vertex_index_start++;
		}
		
		unsigned int vertex_index_end = findNextChar(vertex_index_start, token_c_string, token_length, ' ');
		
		float x = parseOBJFloatValue(line, vertex_index_start, vertex_index_end);
		
		vertex_index_start = vertex_index_end + 1;
		vertex_index_end = findNextChar(vertex_index_start, token_c_string, token_length, ' ');
		
		float y = parseOBJFloatValue(line, vertex_index_start, vertex_index_end);
		
		return {x,y};
	}
	
	static bool compareOBJIndexPointer(const OBJIndex* a, const OBJIndex* b)
	{
		return a->vertex_index < b->vertex_index;
	}
	
	static inline unsigned int findNextChar(unsigned int start, const char* str, unsigned int length, char token)
	{
		unsigned int result = start;
		while(result < length)
		{
			result++;
			if(str[result] == token)
				break;
		}
		
		return result;
	}
	
	static inline unsigned int parseOBJIndexValue(const std::string& token, unsigned int start, unsigned int end)
	{
		return atoi(token.substr(start, end - start).c_str()) - 1;
	}
	
	static inline float parseOBJFloatValue(const std::string& token, unsigned int start, unsigned int end)
	{
		return atof(token.substr(start, end - start).c_str());
	}
}

Font::Font(const std::string& font_path, int pixel_height): font_path(font_path), pixel_height(pixel_height), font_handle(TTF_OpenFont(this->font_path.c_str(), this->pixel_height)){}

Font::Font(const Font& copy): Font(copy.font_path, copy.pixel_height){}
Font::Font(Font&& move): font_path(move.font_path), pixel_height(move.pixel_height), font_handle(move.font_handle)
{
	move.font_handle = nullptr;
}

Font& Font::operator=(Font&& rhs)
{
	this->font_path = rhs.font_path;
	this->pixel_height = rhs.pixel_height;
	this->font_handle = rhs.font_handle;
	rhs.font_handle = nullptr;
	return *this;
}

Font::~Font()
{
	if(this->font_handle == nullptr) // if its been moved, dont try and delete it'll crash if you do
		return;
	TTF_CloseFont(this->font_handle);
	this->font_handle = nullptr;
}

TTF_Font* Font::getFontHandle() const
{
	return this->font_handle;
}

TTF_Font*& Font::getFontHandleR()
{
	return this->font_handle;
}