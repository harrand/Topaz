#include "graphics/texture.hpp"

Texture::Texture():Texture(0, 0, false){}

// This is private, to use this constructor, call it via Texture(int width, int height)
Texture::Texture(int width, int height, bool initialise_handle, tz::graphics::TextureComponent texture_component, bool gamma_corrected): texture_handle(0), width(width), height(height), components(0), texture_component(texture_component), gamma_corrected(gamma_corrected), bitmap({})
{
	if(initialise_handle)
	{
		// Generates a new texture, and just fills it with zeroes if specified.
		glGenTextures(1, &(this->texture_handle));
		glBindTexture(GL_TEXTURE_2D, this->texture_handle);
		switch(this->texture_component)
		{
			default:
			case tz::graphics::TextureComponent::COLOUR_TEXTURE:
				this->components = 4;
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, this->gamma_corrected ? GL_SRGB8_ALPHA8 : GL_RGBA8, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				break;
			case tz::graphics::TextureComponent::HDR_COLOUR_TEXTURE:
				this->components = 4;
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, this->width, this->height, 0, GL_RGBA, GL_FLOAT, NULL);
				break;
			case tz::graphics::TextureComponent::DEPTH_TEXTURE:
				this->components = 1;
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, this->width, this->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
				break;
		}
		// Unbind the texture.
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

Texture::Texture(int width, int height, tz::graphics::TextureComponent texture_component, bool gamma_corrected): Texture(width, height, true, texture_component, gamma_corrected){}

Texture::Texture(std::string filename, bool mipmapping, bool gamma_corrected): Texture(Image{filename}, mipmapping, gamma_corrected) {}

Texture::Texture(const Image& image, bool mipmapping, bool gamma_corrected): texture_handle(0), width(0), height(0), components(4), gamma_corrected(gamma_corrected), bitmap({})
{
	this->width = image.get_width();
	this->height = image.get_height();

	glGenTextures(1, &(this->texture_handle));
	// Let OGL know it's just a 2d texture.
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	if(mipmapping)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    topaz_assert(!image.get_data().empty(), "Texture::Texture(Image, ...): Image needs to have data! This has no data!");
    topaz_assert(image.get_data().get_element_capacity() == (this->width * this->height * 4u), "Texture::Texture(Image, ...): Image data size (", image.get_data().get_element_capacity(), ") does not match its expected size (", this->width, "*", this->height, "*4 == ", (this->width * this->height * 4), ").");

	glTexImage2D(GL_TEXTURE_2D, 0, this->gamma_corrected ? GL_SRGB8_ALPHA8 : GL_RGBA8, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image.get_data()[0]);
	if(mipmapping)
		glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::Texture(const Font& font, const std::string& text, SDL_Color foreground_colour): Texture()
{
	if(font.font_handle == NULL)
	{
		tz::debug::print("Texture::Texture(Font, ...): Error: Texture attempted to load from an invalid font. Error: \"", TTF_GetError(), "\".\n");
		this->bitmap = {};
		return;
	}
	SDL_Surface* text_surface = TTF_RenderUTF8_Blended(font.font_handle, text.c_str(), foreground_colour);
	GLenum texture_format, bytes_per_pixel = text_surface->format->BytesPerPixel;
	constexpr long mask = 0x000000ff;
	this->width = text_surface->w;
	this->height = text_surface->h;
	if(bytes_per_pixel == 4) // alpha
	{
		if(text_surface->format->Rmask == mask)
			texture_format = GL_RGBA8;
		else
			texture_format = GL_BGRA;
	}
	else
	{	// no alpha
		if(text_surface->format->Rmask == mask)
			texture_format = GL_RGB;
		else
			texture_format = GL_BGR;
	}
	glGenTextures(1, &(this->texture_handle));
	// Let OGL know it's just a 2d texture.
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, bytes_per_pixel, text_surface->w, text_surface->h, 0, texture_format, GL_UNSIGNED_BYTE, text_surface->pixels);
	// if ctor parameter said to store the bitmap in RAM, then dewit. otherwise dont bother because it eats lots of ram and its in VRAM anyway
	// Store necessary data in this bitmap.
	auto pixel_data = reinterpret_cast<unsigned char*>(text_surface->pixels);
	this->bitmap = Bitmap<PixelRGBA>();
	this->bitmap.pixels.reserve(static_cast<std::size_t>(std::abs(this->width * this->height)));
	for(std::size_t i = 3; i < static_cast<std::size_t>(std::abs(this->width * this->height)); i += 4) {
		switch (texture_format)
		{
			case GL_RGBA8:
			default:
				this->bitmap.pixels.emplace_back(
						PixelRGBA{pixel_data[i - 3], pixel_data[i - 2], pixel_data[i - 1], pixel_data[i]});
				break;
			case GL_BGRA:
				this->bitmap.pixels.emplace_back(
						PixelRGBA{pixel_data[i - 1], pixel_data[i - 2], pixel_data[i - 3], pixel_data[i]});
				break;
			case GL_RGB:
				this->bitmap.pixels.emplace_back(
						PixelRGBA{pixel_data[i - 3], pixel_data[i - 2], pixel_data[i - 1], 255});
				break;
			case GL_BGR:
				this->bitmap.pixels.emplace_back(
						PixelRGBA{pixel_data[i - 1], pixel_data[i - 2], pixel_data[i - 3], 255});
				break;
		}
	}
	SDL_FreeSurface(text_surface);
}

Texture::Texture(aiTexture* texture): Texture(Image{texture}) {}

Texture::Texture(const Texture& copy): Texture(copy.width, copy.height, copy.get_texture_component(), copy.gamma_corrected)
{
	this->components = copy.components;
	this->bitmap = copy.bitmap;
	tz::graphics::asset::unbind_texture();
	/*
	std::cout << "width difference = " << this->width - copy.width << ", height diference = " << this->height - copy.height << "\n";
	std::cout << "error before copy: " << glGetError() << "\n";
	GLint format;
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0 ,GL_TEXTURE_INTERNAL_FORMAT, &format);
	std::cout << "copy-to internal format = " << format << "\n";
	glBindTexture(GL_TEXTURE_2D, copy.texture_handle);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0 ,GL_TEXTURE_INTERNAL_FORMAT, &format);
	std::cout << "copy-from internal format = " << format << "\n";
	*/
	glCopyImageSubData(copy.texture_handle, GL_TEXTURE_2D, 0, 0, 0, 0, this->texture_handle, GL_TEXTURE_2D, 0, 0, 0, 0, copy.width, copy.height, 1);
	//std::cout << "texture copy error: " << glGetError() << "\n";
}

Texture::Texture(Texture&& move): texture_handle(move.texture_handle), width(move.width), height(move.height), components(move.components), gamma_corrected(move.gamma_corrected)
{
	move.texture_handle = 0;
}

Texture::~Texture()
{
	// glDeleteTextures silently ignores 0 as a texture_handle so this should not cause problems if just moved. also due to this reason doesnt crash if the texture used the default constructor
	glDeleteTextures(1, &(this->texture_handle));
}

Texture& Texture::operator=(Texture rhs)
{
	Texture::swap(*this, rhs);
	return *this;
}

/*
Texture& Texture::operator=(Texture&& rhs)
{
	glDeleteTextures(1, &(this->texture_handle));
	this->texture_handle = rhs.texture_handle;
	this->width = rhs.width;
	this->height = rhs.height;
	this->components = rhs.components;
	this->texture_component = rhs.texture_component;
	this->gamma_corrected = rhs.gamma_corrected;
	this->bitmap = rhs.bitmap;
	rhs.texture_handle = 0;
	return *this;
}
 */

void Texture::bind(Shader* shader, unsigned int id, const std::string& sampler_name) const
{
	this->bind_with_string(shader, id, sampler_name);
}

int Texture::get_width() const
{
	return this->width;
}

int Texture::get_height() const
{
	return this->height;
}

tz::graphics::MipmapType Texture::get_mipmap_type() const
{
	GLint flag;
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &flag);
	glBindTexture(GL_TEXTURE_2D, 0);
	switch(flag)
	{
		case static_cast<GLint>(tz::graphics::MipmapType::NEAREST):
			return tz::graphics::MipmapType::NEAREST;
		case static_cast<GLint>(tz::graphics::MipmapType::LINEAR):
			return tz::graphics::MipmapType::LINEAR;
		case static_cast<GLint>(tz::graphics::MipmapType::NEAREST_MULTIPLE):
			return tz::graphics::MipmapType::NEAREST_MULTIPLE;
		case static_cast<GLint>(tz::graphics::MipmapType::LINEAR_MULTIPLE):
			return tz::graphics::MipmapType::LINEAR_MULTIPLE;
		default:
			return tz::graphics::MipmapType::NONE;
	}
}

bool Texture::has_mipmap() const
{
	return this->get_mipmap_type() != tz::graphics::MipmapType::NONE;
}

const Bitmap<PixelRGBA>& Texture::get_bitmap() const
{
	return this->bitmap;
}

tz::graphics::TextureComponent Texture::get_texture_component() const
{
	return this->texture_component;
}

bool Texture::operator==(const Texture& rhs) const
{
	return this->texture_handle == rhs.texture_handle;
}

void Texture::bind_with_string(Shader* shader, unsigned int id, const std::string& sampler_uniform_name) const
{
	if(id > 31)
	{
		tz::debug::print("Texture::bind_with_string(...): Error: Texture bind ID ", id, " is invalid. Must be between 1-31\n");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	shader->set_uniform<int>(sampler_uniform_name, id);
}

void Texture::swap(Texture& lhs, Texture& rhs)
{
	std::swap(lhs.texture_handle, rhs.texture_handle);
	std::swap(lhs.width, rhs.width);
	std::swap(lhs.height, rhs.height);
	std::swap(lhs.components, rhs.components);
	std::swap(lhs.texture_component, rhs.texture_component);
	std::swap(lhs.gamma_corrected, rhs.gamma_corrected);
	std::swap(lhs.bitmap, rhs.bitmap);
}

NormalMap::NormalMap(std::string filename): Texture(filename, false, false){}
NormalMap::NormalMap(aiTexture* assimp_texture): Texture(assimp_texture){}

void NormalMap::bind(Shader* shader, unsigned int id, const std::string& sampler_name) const
{
	this->bind_with_string(shader, id, sampler_name);
	shader->set_uniform<bool>("has_normal_map", true);
}

ParallaxMap::ParallaxMap(std::string filename, float multiplier, float offset): Texture(filename, false, false), multiplier(multiplier), bias(this->multiplier / 2.0f * offset){}

void ParallaxMap::bind(Shader* shader, unsigned int id, const std::string& sampler_name) const
{
	this->bind_with_string(shader, id, sampler_name);
	shader->set_uniform<bool>("has_parallax_map", true);
	shader->set_uniform<float>("parallax_multiplier", this->multiplier);
	shader->set_uniform<float>("parallax_bias", this->bias);
}

DisplacementMap::DisplacementMap(std::string filename, float displacement_factor): Texture(filename, false, false), displacement_factor(displacement_factor){}
DisplacementMap::DisplacementMap(Bitmap<PixelDepth> height_map, float displacement_factor): Texture(height_map), displacement_factor(displacement_factor){}
DisplacementMap::DisplacementMap(aiTexture* assimp_texture): Texture(assimp_texture){}

void DisplacementMap::bind(Shader* shader, unsigned int id, const std::string& sampler_name) const
{
	this->bind_with_string(shader, id, sampler_name);
	shader->set_uniform<bool>("has_displacement_map", true);
	shader->set_uniform<float>("displacement_factor", this->displacement_factor);
}

SpecularMap::SpecularMap(std::string filename): Texture(filename, false, false){}
SpecularMap::SpecularMap(aiTexture* assimp_texture): Texture(assimp_texture){}

void SpecularMap::bind(Shader* shader, unsigned int id, const std::string& sampler_name) const
{
	this->bind_with_string(shader, id, sampler_name);
	shader->set_uniform<bool>("has_specular_map", true);
}

EmissiveMap::EmissiveMap(std::string filename): Texture(filename, false, false){}
EmissiveMap::EmissiveMap(aiTexture* assimp_texture): Texture(assimp_texture){}

void EmissiveMap::bind(Shader* shader, unsigned int id, const std::string& sampler_name) const
{
	this->bind_with_string(shader, id, sampler_name);
	shader->set_uniform<bool>("has_emissive_map", true);
}

CubeMap::CubeMap(std::string right_texture, std::string left_texture, std::string top_texture, std::string bottom_texture, std::string back_texture, std::string front_texture): right_texture(std::move(right_texture)), left_texture(std::move(left_texture)), top_texture(std::move(top_texture)), bottom_texture(std::move(bottom_texture)), back_texture(std::move(back_texture)), front_texture(std::move(front_texture))
{
	glGenTextures(1, &(this->texture_handle));
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->texture_handle);
	std::vector<Image> face_data = this->load_textures();
	for(std::size_t i = 0; i < face_data.size(); i++)
	{
		const Image& face = face_data[i];
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, face.get_width(), face.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &face.get_data()[0]);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

CubeMap::CubeMap(std::string texture_directory, std::string skybox_name, std::string skybox_image_file_extension): CubeMap(texture_directory + skybox_name + "_rt" + skybox_image_file_extension, texture_directory + skybox_name + "_lf" + skybox_image_file_extension, texture_directory + skybox_name + "_up" + skybox_image_file_extension, texture_directory + skybox_name + "_dn" + skybox_image_file_extension, texture_directory + skybox_name + "_bk" + skybox_image_file_extension, texture_directory + skybox_name + "_ft" + skybox_image_file_extension){}

CubeMap::CubeMap(const CubeMap& copy): CubeMap(copy.right_texture, copy.left_texture, copy.top_texture, copy.bottom_texture, copy.back_texture, copy.front_texture){}

CubeMap::CubeMap(CubeMap&& move): texture_handle(move.texture_handle), right_texture(move.right_texture), left_texture(move.left_texture), top_texture(move.top_texture), bottom_texture(move.bottom_texture), back_texture(move.back_texture), front_texture(move.front_texture)
{
	move.texture_handle = 0;
}

CubeMap::~CubeMap()
{
	glDeleteTextures(1, &(this->texture_handle));
}

void CubeMap::bind(Shader* shader, unsigned int id) const
{
	if(id > 31)
	{
		tz::debug::print("CubeMap::bind(...): Error: CubeMap bind ID ", id, " is invalid. Must be between 1-31.\n");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->texture_handle);
	shader->set_uniform<int>("cube_map_sampler", id);
}

std::vector<Image> CubeMap::load_textures()
{
	std::vector<Image> image_data;
	image_data.reserve(6);
	image_data.emplace_back(this->right_texture.c_str());
	image_data.emplace_back(this->left_texture.c_str());
	image_data.emplace_back(this->top_texture.c_str());
	image_data.emplace_back(this->bottom_texture.c_str());
	image_data.emplace_back(this->back_texture.c_str());
	image_data.emplace_back(this->front_texture.c_str());
	return image_data;
}

namespace tz::graphics::height_map
{
	DisplacementMap generate_smooth_noise(std::size_t width, std::size_t height, float displacement_factor, SmoothNoise noise_function)
	{
		std::vector<PixelDepth> pixels;
		pixels.resize(width * height, PixelDepth{0.0f});
		for(std::size_t x = 0; x < width; x++)
		{
			for(std::size_t y = 0; y < height; y++)
			{
				auto& pixel = pixels[x + y * height];
				pixel.data.underlying_data = {noise_function(x, y)};
			}
		}
		return {Bitmap<PixelDepth>{pixels, width, height}, displacement_factor};
	}

	DisplacementMap generate_cosine_noise(std::size_t width, std::size_t height, float displacement_factor, float smoothness, CosineNoise noise_function)
	{
		std::vector<PixelDepth> pixels;
		pixels.resize(width * height, PixelDepth{0.0f});
		for(std::size_t i = 0; i < width * height; i++)
		{
			std::size_t row = i / width;
			std::size_t column = i % width;
			pixels[i].data.underlying_data = {noise_function(column, row, smoothness)};
		}
		return {Bitmap<PixelDepth>{pixels, width, height}, displacement_factor};
	}
}