#include "graphics.hpp"

Font::Font(const std::string& font_path, int pixel_height): font_path(font_path), pixel_height(pixel_height), font_handle(TTF_OpenFont(this->font_path.c_str(), this->pixel_height)){}

Font::Font(const Font& copy): Font(copy.font_path, copy.pixel_height){}
Font::Font(Font&& move): font_path(move.font_path), pixel_height(move.pixel_height), font_handle(move.font_handle)
{
	move.font_handle = nullptr;
}

Font::~Font()
{
	if(this->font_handle == nullptr) // if its been moved, dont try and delete it'll crash if you do
		return;
	TTF_CloseFont(this->font_handle);
	this->font_handle = nullptr;
}

Font& Font::operator=(Font&& rhs)
{
	this->font_path = rhs.font_path;
	this->pixel_height = rhs.pixel_height;
	this->font_handle = rhs.font_handle;
	rhs.font_handle = nullptr;
	return *this;
}

int Font::get_pixel_height() const
{
	return this->pixel_height;
}

const std::string& Font::get_path() const
{
	return this->font_path;
}

Vertex::Vertex(Vector3F position, Vector2F texture_coordinate, Vector3F normal): position(std::move(position)), texture_coordinate(std::move(texture_coordinate)), normal(std::move(normal)){}

PixelRGBA::PixelRGBA(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha): data(red, green, blue, alpha){}

namespace tz::graphics::model
{
	static bool compare_obj_index_pointer(const OBJIndex* a, const OBJIndex* b);
	static inline unsigned int find_next_char(unsigned int start, const char* str, unsigned int length, char token);
	static inline unsigned int parse_obj_index_value(const std::string& token, unsigned int start, unsigned int end);
	static inline float parse_obj_float_value(const std::string& token, unsigned int start, unsigned int end);
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
							this->uvs.push_back(parse_obj_vector_2f(line));
						else if(line_c_str[1] == 'n')
							this->normals.push_back(parse_obj_vector_3f(line));
						else if(line_c_str[1] == ' ' || line_c_str[1] == '\t')
							this->vertices.push_back(parse_obj_vector_3f(line));
					break;
					case 'f':
						create_obj_face(line);
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
	
	void IndexedModel::calculate_normals()
	{
		for(std::size_t i = 0; i < indices.size(); i += 3)
		{
			int i_0 = indices[i];
			int i_1 = indices[i + 1];
			int i_2 = indices[i + 2];
	
			Vector3F v_1 = (positions[i_1] - positions[i_0]);
			Vector3F v_2 = (positions[i_2] - positions[i_0]);
			
			Vector3F normal = (v_1.cross(v_2)).normalised();//glm::normalize(glm::cross(v_1, v_2));
				
			normals[i_0] = (normals[i_0] + normal);
			normals[i_1] = (normals[i_1] + normal);
			normals[i_2] = (normals[i_2] + normal);
		}
		
		for(std::size_t i = 0; i < positions.size(); i++)
			normals[i] = normals[i].normalised();
	}
	
	void IndexedModel::calculate_tangents()
	{
		tangents.clear();
		tangents.resize(positions.size());
		
		for(std::size_t i = 0; i < indices.size(); i += 3)
		{
			int i_0 = indices[i];
			int i_1 = indices[i + 1];
			int i_2 = indices[i + 2];
			Vector3F edge_1 = (positions[i_1] - positions[i_0]);
			Vector3F edge_2 = (positions[i_2] - positions[i_0]);
			float delta_u_1 = texcoords[i_1].x - texcoords[i_0].x;
			float delta_u_2 = texcoords[i_2].x - texcoords[i_0].x;
			float delta_v_1 = texcoords[i_1].y - texcoords[i_0].y;
			float delta_v_2 = texcoords[i_2].y - texcoords[i_0].y;
			float dividend = (delta_u_1 * delta_v_2 - delta_u_2 * delta_v_1);
			float f = dividend == 0.0f ? 0.0f : 1.0f/dividend;
			Vector3F tangent(f * (delta_v_2 * edge_1.x - delta_v_1 * edge_2.x), f * (delta_v_2 * edge_1.y - delta_v_1 * edge_2.y), f * (delta_v_2 * edge_1.z - delta_v_1 * edge_2.z));
			tangents[i_0] += tangent;
			tangents[i_1] += tangent;
			tangents[i_2] += tangent;
		}
		for(Vector3F& tangent : tangents)
			tangent = tangent.normalised();
	}
	
	IndexedModel OBJModel::to_indexed_model()
	{
		IndexedModel result;
		IndexedModel normal_model;
		std::size_t number_of_indices = this->obj_indices.size();
		std::vector<OBJIndex*> index_lookup;
		for(std::size_t i = 0; i < number_of_indices; i++)
			index_lookup.push_back(&obj_indices[i]);
		std::sort(index_lookup.begin(), index_lookup.end(), compare_obj_index_pointer);
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
				normal_model.tangents.emplace_back(Vector3F());
			}
			else
				normal_model_index = it->second;
			unsigned int previous_vertex_location = find_last_vertex_index(index_lookup, current_index, result);
			if(previous_vertex_location == static_cast<unsigned int>(-1))
			{
				result_model_index = result.positions.size();
			
				result.positions.push_back(current_position);
				result.texcoords.push_back(current_texture_coordinate);
				result.normals.push_back(current_normal);
				result.tangents.emplace_back(Vector3F());
			}
			else
				result_model_index = previous_vertex_location;
			normal_model.indices.push_back(normal_model_index);
			result.indices.push_back(result_model_index);
			index_map.emplace(result_model_index, normal_model_index);
		}
		if(!has_normals)
		{
			normal_model.calculate_normals();
			
			for(std::size_t i = 0; i < result.positions.size(); i++)
				result.normals[i] = normal_model.normals[index_map[i]];
		}
		// normal model has smooth faces.
		normal_model.calculate_tangents();
		for(std::size_t i = 0; i < result.positions.size(); i++)
				result.tangents[i] += normal_model.tangents[index_map[i]];
		return result;
	}
	
	unsigned int OBJModel::find_last_vertex_index(const std::vector<OBJIndex*>& index_lookup, const OBJIndex* current_index, const IndexedModel& result)
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
	
	void OBJModel::create_obj_face(const std::string& line)
	{
		std::vector<std::string> tokens = tz::util::string::split_string(line, ' ');
	
		this->obj_indices.push_back(parse_obj_index(tokens[1], &this->has_uvs, &this->has_normals));
		this->obj_indices.push_back(parse_obj_index(tokens[2], &this->has_uvs, &this->has_normals));
		this->obj_indices.push_back(parse_obj_index(tokens[3], &this->has_uvs, &this->has_normals));
	
		if(tokens.size() > 4)
		{
			this->obj_indices.push_back(parse_obj_index(tokens[1], &this->has_uvs, &this->has_normals));
			this->obj_indices.push_back(parse_obj_index(tokens[3], &this->has_uvs, &this->has_normals));
			this->obj_indices.push_back(parse_obj_index(tokens[4], &this->has_uvs, &this->has_normals));
		}
	}
	
	OBJIndex OBJModel::parse_obj_index(const std::string& token, bool* has_uvs, bool* has_normals)
	{
		std::size_t token_length = token.length();
		const char* token_c_string = token.c_str();
		
		unsigned int vertex_index_start = 0;
		unsigned int vertex_index_end = find_next_char(vertex_index_start, token_c_string, token_length, '/');
		
		OBJIndex result;
		result.vertex_index = parse_obj_index_value(token, vertex_index_start, vertex_index_end);
		result.uv_index = 0;
		result.normal_index = 0;
		
		if(vertex_index_end >= token_length)
			return result;
		
		vertex_index_start = vertex_index_end + 1;
		vertex_index_end = find_next_char(vertex_index_start, token_c_string, token_length, '/');
		
		result.uv_index = parse_obj_index_value(token, vertex_index_start, vertex_index_end);
		*has_uvs = true;
		
		if(vertex_index_end >= token_length)
			return result;
		
		vertex_index_start = vertex_index_end + 1;
		vertex_index_end = find_next_char(vertex_index_start, token_c_string, token_length, '/');
		
		result.normal_index = parse_obj_index_value(token, vertex_index_start, vertex_index_end);
		*has_normals = true;
		
		return result;
	}
	
	Vector3F OBJModel::parse_obj_vector_3f(const std::string& line) 
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
		
		unsigned int vertex_index_end = find_next_char(vertex_index_start, token_c_string, token_length, ' ');
		
		float x = parse_obj_float_value(line, vertex_index_start, vertex_index_end);
		
		vertex_index_start = vertex_index_end + 1;
		vertex_index_end = find_next_char(vertex_index_start, token_c_string, token_length, ' ');
		
		float y = parse_obj_float_value(line, vertex_index_start, vertex_index_end);
		
		vertex_index_start = vertex_index_end + 1;
		vertex_index_end = find_next_char(vertex_index_start, token_c_string, token_length, ' ');
		
		float z = parse_obj_float_value(line, vertex_index_start, vertex_index_end);
		
		return {x,y,z};
	}
	
	Vector2F OBJModel::parse_obj_vector_2f(const std::string& line)
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
		
		unsigned int vertex_index_end = find_next_char(vertex_index_start, token_c_string, token_length, ' ');
		
		float x = parse_obj_float_value(line, vertex_index_start, vertex_index_end);
		
		vertex_index_start = vertex_index_end + 1;
		vertex_index_end = find_next_char(vertex_index_start, token_c_string, token_length, ' ');
		
		float y = parse_obj_float_value(line, vertex_index_start, vertex_index_end);
		
		return {x,y};
	}
	
	static bool compare_obj_index_pointer(const OBJIndex* a, const OBJIndex* b)
	{
		return a->vertex_index < b->vertex_index;
	}
	
	static inline unsigned int find_next_char(unsigned int start, const char* str, unsigned int length, char token)
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
	
	static inline unsigned int parse_obj_index_value(const std::string& token, unsigned int start, unsigned int end)
	{
		return atoi(token.substr(start, end - start).c_str()) - 1;
	}
	
	static inline float parse_obj_float_value(const std::string& token, unsigned int start, unsigned int end)
	{
		return atof(token.substr(start, end - start).c_str());
	}
}