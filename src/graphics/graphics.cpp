#include "graphics.hpp"
#include <map>

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

Font& Font::operator=(Font rhs)
{
	std::swap(this->font_path, rhs.font_path);
	std::swap(this->pixel_height, rhs.pixel_height);
	std::swap(this->font_handle, rhs.font_handle);
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

Vertex::Vertex(Vector3F position, Vector2F texture_coordinate, Vector3F normal, Vector3F tangent): position(std::move(position)), texture_coordinate(std::move(texture_coordinate)), normal(std::move(normal)), tangent(std::move(tangent)){}

bool Vertex::operator==(const Vertex& rhs) const
{
    return
            this->position == rhs.position &&
            this->texture_coordinate == rhs.texture_coordinate &&
            this->normal == rhs.normal &&
            this->tangent == rhs.tangent;
}