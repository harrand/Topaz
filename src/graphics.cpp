#include "graphics.hpp"
#include <map>

static bool CompareOBJIndexPtr(const OBJIndex* a, const OBJIndex* b);
static inline unsigned int FindNextChar(unsigned int start, const char* str, unsigned int length, char token);
static inline unsigned int parseOBJIndexValue(const std::string& token, unsigned int start, unsigned int end);
static inline float ParseOBJFloatValue(const std::string& token, unsigned int start, unsigned int end);
static inline std::vector<std::string> SplitString(const std::string &s, char delim);

OBJModel::OBJModel(const std::string& file_name)
{
	has_uvs = false;
	has_normals = false;
    std::ifstream file;
    file.open(file_name.c_str());

    std::string line;
    if(file.is_open())
    {
        while(file.good())
        {
            getline(file, line);
        
            unsigned int lineLength = line.length();
            
            if(lineLength < 2)
                continue;
            
            const char* lineCStr = line.c_str();
            
            switch(lineCStr[0])
            {
                case 'v':
                    if(lineCStr[1] == 't')
                        this->uvs.push_back(parseOBJVector2F(line));
                    else if(lineCStr[1] == 'n')
                        this->normals.push_back(parseOBJVector3F(line));
                    else if(lineCStr[1] == ' ' || lineCStr[1] == '\t')
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
        std::cerr << "Unable to load mesh: " << file_name << std::endl;
    }
}

void IndexedModel::calcNormals()
{
    for(unsigned int i = 0; i < indices.size(); i += 3)
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
    
    for(unsigned int i = 0; i < positions.size(); i++)
        normals[i] = normals[i].normalised();
}

void IndexedModel::calcTangents()
{
    for(unsigned int i = 0; i < indices.size(); i += 3)
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
		//std::cout << "Adding the tangent trio:\n";
		//std::cout << "	[" << tangent.getX() << ", " << tangent.getY() << ", " << tangent.getZ() << "]\n";
		tangents.at(i0) += tangent;
		tangents.at(i1) += tangent;
		tangents.at(i2) += tangent;
		//std::cout << "		Added.\n";
    }
    
    for(unsigned int i = 0; i < tangents.size(); i++)
        tangents[i] = tangents[i].normalised();
}

IndexedModel OBJModel::toIndexedModel()
{
    IndexedModel result;
    IndexedModel normalModel;
    
    unsigned int numIndices = obj_indices.size();
	normalModel.tangents.resize(numIndices);
	result.tangents.resize(numIndices);
    
    std::vector<OBJIndex*> index_lookup;
    
    for(unsigned int i = 0; i < numIndices; i++)
        index_lookup.push_back(&obj_indices[i]);
    
    std::sort(index_lookup.begin(), index_lookup.end(), CompareOBJIndexPtr);
    
    std::map<OBJIndex, unsigned int> normalModelIndexMap;
    std::map<unsigned int, unsigned int> indexMap;
    
    for(unsigned int i = 0; i < numIndices; i++)
    {
        OBJIndex* current_index = &obj_indices[i];
        
        Vector3F currentPosition = vertices[current_index->vertex_index];
        Vector2F currentTexCoord;
        Vector3F currentNormal;
        
        if(has_uvs)
            currentTexCoord = uvs[current_index->uv_index];
        else
            currentTexCoord = Vector2F(0,0);
            
        if(has_normals)
            currentNormal = normals[current_index->normal_index];
        else
            currentNormal = Vector3F(0,0,0);
        
        unsigned int normalModelIndex;
        unsigned int resultModelIndex;
        
        //Create model to properly generate normals on
        std::map<OBJIndex, unsigned int>::iterator it = normalModelIndexMap.find(*current_index);
        if(it == normalModelIndexMap.end())
        {
            normalModelIndex = normalModel.positions.size();
        
            normalModelIndexMap.insert(std::pair<OBJIndex, unsigned int>(*current_index, normalModelIndex));
            normalModel.positions.push_back(currentPosition);
            normalModel.texcoords.push_back(currentTexCoord);
            normalModel.normals.push_back(currentNormal);
        }
        else
            normalModelIndex = it->second;
        
        //Create model which properly separates texture coordinates
        unsigned int previousVertexLocation = findLastVertexIndex(index_lookup, current_index, result);
        
        if(previousVertexLocation == (unsigned int)-1)
        {
            resultModelIndex = result.positions.size();
        
            result.positions.push_back(currentPosition);
            result.texcoords.push_back(currentTexCoord);
            result.normals.push_back(currentNormal);
        }
        else
            resultModelIndex = previousVertexLocation;
        
        normalModel.indices.push_back(normalModelIndex);
        result.indices.push_back(resultModelIndex);
        indexMap.insert(std::pair<unsigned int, unsigned int>(resultModelIndex, normalModelIndex));
    }
    
    if(!has_normals)
    {
        normalModel.calcNormals();
        
        for(unsigned int i = 0; i < result.positions.size(); i++)
            result.normals[i] = normalModel.normals[indexMap[i]];
    }
	
	
	normalModel.calcTangents();
	for(unsigned int i = 0; i < result.tangents.size(); i++)
		result.tangents.at(i) = normalModel.tangents[indexMap[i]];
	
    
    return result;
}

unsigned int OBJModel::findLastVertexIndex(const std::vector<OBJIndex*>& index_lookup, const OBJIndex* current_index, const IndexedModel& result)
{
    unsigned int start = 0;
    unsigned int end = index_lookup.size();
    unsigned int current = (end - start) / 2 + start;
    unsigned int previous = start;
    
    while(current != previous)
    {
        OBJIndex* testIndex = index_lookup[current];
        
        if(testIndex->vertex_index == current_index->vertex_index)
        {
            unsigned int countStart = current;
        
            for(unsigned int i = 0; i < current; i++)
            {
                OBJIndex* possibleIndex = index_lookup[current - i];
                
                if(possibleIndex == current_index)
                    continue;
                    
                if(possibleIndex->vertex_index != current_index->vertex_index)
                    break;
                    
                countStart--;
            }
            
            for(unsigned int i = countStart; i < index_lookup.size() - countStart; i++)
            {
                OBJIndex* possibleIndex = index_lookup[current + i];
                
                if(possibleIndex == current_index)
                    continue;
                    
                if(possibleIndex->vertex_index != current_index->vertex_index)
                    break;
                else if((!has_uvs || possibleIndex->uv_index == current_index->uv_index) 
                    && (!has_normals || possibleIndex->normal_index == current_index->normal_index))
                {
                    Vector3F currentPosition = vertices[current_index->vertex_index];
                    Vector2F currentTexCoord;
                    Vector3F currentNormal;
                    
                    if(has_uvs)
                        currentTexCoord = uvs[current_index->uv_index];
                    else
                        currentTexCoord = Vector2F(0,0);
                        
                    if(has_normals)
                        currentNormal = normals[current_index->normal_index];
                    else
                        currentNormal = Vector3F(0,0,0);
                    
                    for(unsigned int j = 0; j < result.positions.size(); j++)
                    {
                        if(currentPosition == result.positions[j] 
                            && ((!has_uvs || currentTexCoord == result.texcoords[j])
                            && (!has_normals || currentNormal == result.normals[j])))
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
            if(testIndex->vertex_index < current_index->vertex_index)
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
    std::vector<std::string> tokens = SplitString(line, ' ');

    this->obj_indices.push_back(parseOBJIndex(tokens[1], &this->has_uvs, &this->has_normals));
    this->obj_indices.push_back(parseOBJIndex(tokens[2], &this->has_uvs, &this->has_normals));
    this->obj_indices.push_back(parseOBJIndex(tokens[3], &this->has_uvs, &this->has_normals));

    if((int)tokens.size() > 4)
    {
        this->obj_indices.push_back(parseOBJIndex(tokens[1], &this->has_uvs, &this->has_normals));
        this->obj_indices.push_back(parseOBJIndex(tokens[3], &this->has_uvs, &this->has_normals));
        this->obj_indices.push_back(parseOBJIndex(tokens[4], &this->has_uvs, &this->has_normals));
    }
}

OBJIndex OBJModel::parseOBJIndex(const std::string& token, bool* has_uvs, bool* has_normals)
{
    unsigned int tokenLength = token.length();
    const char* tokenString = token.c_str();
    
    unsigned int vertIndexStart = 0;
    unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');
    
    OBJIndex result;
    result.vertex_index = parseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
    result.uv_index = 0;
    result.normal_index = 0;
    
    if(vertIndexEnd >= tokenLength)
        return result;
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');
    
    result.uv_index = parseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
    *has_uvs = true;
    
    if(vertIndexEnd >= tokenLength)
        return result;
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');
    
    result.normal_index = parseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
    *has_normals = true;
    
    return result;
}

Vector3F OBJModel::parseOBJVector3F(const std::string& line) 
{
    unsigned int tokenLength = line.length();
    const char* tokenString = line.c_str();
    
    unsigned int vertIndexStart = 2;
    
    while(vertIndexStart < tokenLength)
    {
        if(tokenString[vertIndexStart] != ' ')
            break;
        vertIndexStart++;
    }
    
    unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');
    
    float x = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');
    
    float y = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');
    
    float z = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    return Vector3F(x,y,z);

    //Vector3F(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()))
}

Vector2F OBJModel::parseOBJVector2F(const std::string& line)
{
    unsigned int tokenLength = line.length();
    const char* tokenString = line.c_str();
    
    unsigned int vertIndexStart = 3;
    
    while(vertIndexStart < tokenLength)
    {
        if(tokenString[vertIndexStart] != ' ')
            break;
        vertIndexStart++;
    }
    
    unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');
    
    float x = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');
    
    float y = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    return Vector2F(x,y);
}

static bool CompareOBJIndexPtr(const OBJIndex* a, const OBJIndex* b)
{
    return a->vertex_index < b->vertex_index;
}

static inline unsigned int FindNextChar(unsigned int start, const char* str, unsigned int length, char token)
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

static inline float ParseOBJFloatValue(const std::string& token, unsigned int start, unsigned int end)
{
    return atof(token.substr(start, end - start).c_str());
}

static inline std::vector<std::string> SplitString(const std::string &s, char delim)
{
    std::vector<std::string> elems;
        
    const char* cstr = s.c_str();
    unsigned int strLength = s.length();
    unsigned int start = 0;
    unsigned int end = 0;
        
    while(end <= strLength)
    {
        while(end <= strLength)
        {
            if(cstr[end] == delim)
                break;
            end++;
        }
            
        elems.push_back(s.substr(start, end - start));
        start = end + 1;
        end = start;
    }
        
    return elems;
}