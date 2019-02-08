#include "graphics/graphics.hpp"
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
	return *this;
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

Font::Style Font::get_style() const
{
	return static_cast<Font::Style>(TTF_GetFontStyle(this->font_handle));
}

void Font::set_style(Font::Style style)
{
	TTF_SetFontStyle(this->font_handle, static_cast<int>(style));
}

bool Font::has_outline() const
{
	return this->get_outline_size() == 0;
}

int Font::get_outline_size() const
{
	return TTF_GetFontOutline(this->font_handle);
}

void Font::set_outline_size(int outline)
{
	TTF_SetFontOutline(this->font_handle, outline);
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

PixelRGBA& PixelRGBA::operator=(const Vector<4, unsigned char>& data)
{
	this->data = data;
    for(auto& data_element : this->data.underlying_data)
    {
        data_element = std::clamp<unsigned char>(0, 255, data_element);
    }
	return *this;
}

PixelRGBAHDR& PixelRGBAHDR::operator=(const Vector<4, float>& data)
{
	this->data = data;
    for(auto& data_element : this->data.underlying_data)
    {
        data_element = std::clamp<float>(0.0f, 255.0f, data_element);
    }
	return *this;
}

PixelDepth& PixelDepth::operator=(float depth)
{
	this->data.underlying_data[0] = std::clamp(0.0f, 1.0f, depth);
	return *this;
}