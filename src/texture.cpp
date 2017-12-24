#include "texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture():Texture(0, 0, false){}

// This is private, to use this constructor, call it via Texture(int width, int height)
Texture::Texture(int width, int height, bool initialise_handle): filename({}), texture_handle(0), width(width), height(height), components(0), gamma_corrected(false), bitmap({})
{
	if(initialise_handle)
	{
		// Generates a new texture, and just fills it with zeroes if specified.
		glGenTextures(1, &(this->texture_handle));
		glBindTexture(GL_TEXTURE_2D, this->texture_handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		// Unbind the texture.
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

Texture::Texture(int width, int height): Texture(width, height, true){}

Texture::Texture(std::string filename, bool gamma_corrected, bool store_bitmap): filename(filename), texture_handle(0), width(0), height(0), components(0), gamma_corrected(gamma_corrected), bitmap({})
{
	unsigned char* imgdata = this->load_texture();
	if(imgdata == nullptr)
	{
		tz::util::log::error("Texture from the path: '", filename, "' could not be loaded.");
	}
	
	glGenTextures(1, &(this->texture_handle));
	// Let OGL know it's just a 2d texture.
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, this->gamma_corrected ? GL_SRGB_ALPHA : GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgdata);
	
	if(store_bitmap)
	{
		this->bitmap = Bitmap<PixelRGBA>();
		this->bitmap.value().pixels.reserve(std::abs(this->width * this->height));
		for(std::size_t i = 3; i < std::abs(this->width * this->height); i += 4)
			this->bitmap.value().pixels.emplace_back(PixelRGBA{imgdata[i - 3], imgdata[i - 2], imgdata[i - 1], imgdata[i]});
	}
	this->delete_texture(imgdata);
}

Texture::Texture(const Font& font, const std::string& text, SDL_Color foreground_colour, bool store_bitmap): Texture()
{
	if(font.font_handle == NULL)
		tz::util::log::error("Texture attempted to load from an invalid font. Error: ", TTF_GetError());
	SDL_Surface* text_surface = TTF_RenderUTF8_Blended(font.font_handle, text.c_str(), foreground_colour);
	GLint texture_format, bytes_per_pixel = text_surface->format->BytesPerPixel;
	constexpr long mask = 0x000000ff;
	this->width = text_surface->w;
	this->height = text_surface->h;
	if(bytes_per_pixel == 4) // alpha
	{
		if(text_surface->format->Rmask == mask)
			texture_format = GL_RGBA;
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
	if(store_bitmap)
	{
		unsigned char* pixel_data = reinterpret_cast<unsigned char*>(text_surface->pixels);
		this->bitmap = Bitmap<PixelRGBA>();
		this->bitmap.value().pixels.reserve(std::abs(this->width * this->height));
		for(std::size_t i = 3; i < std::abs(this->width * this->height); i += 4)
			switch(texture_format)
			{
				case GL_RGBA:
					this->bitmap.value().pixels.emplace_back(PixelRGBA{pixel_data[i - 3], pixel_data[i - 2], pixel_data[i - 1], pixel_data[i]});
					break;
				case GL_BGRA:
					this->bitmap.value().pixels.emplace_back(PixelRGBA{pixel_data[i - 1], pixel_data[i - 2], pixel_data[i - 3], pixel_data[i]});
					break;
				case GL_RGB:
					this->bitmap.value().pixels.emplace_back(PixelRGBA{pixel_data[i - 3], pixel_data[i - 2], pixel_data[i - 1], 255});
					break;
				case GL_BGR:
					this->bitmap.value().pixels.emplace_back(PixelRGBA{pixel_data[i - 1], pixel_data[i - 2], pixel_data[i - 3], 255});
					break;
			}
	}
	SDL_FreeSurface(text_surface);
}

Texture::Texture(const Texture& copy): Texture(copy.get_file_name()){}

Texture::Texture(Texture&& move): filename(move.get_file_name()), texture_handle(move.texture_handle), width(move.width), height(move.height), components(move.components)
{
	move.texture_handle = 0;
}

Texture::~Texture()
{
	// glDeleteTextures silently ignores 0 as a texture_handle so this should not cause problems if just moved. also due to this reason doesnt crash if the texture used the default constructor
	glDeleteTextures(1, &(this->texture_handle));
}

Texture& Texture::operator=(Texture&& rhs)
{
	glDeleteTextures(1, &(this->texture_handle));
	this->filename = rhs.filename;
	this->texture_handle = rhs.texture_handle;
	this->width = rhs.width;
	this->height = rhs.height;
	this->components = rhs.components;
	rhs.texture_handle = 0;
	return *this;
}

void Texture::bind(Shader* shader, unsigned int id) const
{
	if(id > 31)
	{
		tz::util::log::error("FrameBuffer bind ID ", id, " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	shader->set_uniform<int>("texture_sampler", id);
}

bool Texture::has_file_name() const
{
	return this->filename.has_value();
}

const std::string& Texture::get_file_name() const
{
	return this->filename.value();
}

int Texture::get_width() const
{
	return this->width;
}

int Texture::get_height() const
{
	return this->height;
}

bool Texture::has_bitmap() const
{
	return this->bitmap.has_value();
}

Bitmap<PixelRGBA> Texture::get_bitmap() const
{
	return this->bitmap.value_or(Bitmap<PixelRGBA>());
}

bool Texture::operator==(const Texture& rhs)
{
	return this->texture_handle == rhs.texture_handle;
}

unsigned char* Texture::load_texture()
{
	if(this->has_file_name())
		return stbi_load((this->get_file_name()).c_str(), &(this->width), &(this->height), &(this->components), 4);
	else
		return nullptr;
}

// Deleting texture as far as stb_image is concerned (We want to leave it alone when it's gone to the GPU)
void Texture::delete_texture(unsigned char* imgdata)
{
	stbi_image_free(imgdata);
}

NormalMap::NormalMap(std::string filename): Texture(filename, false){}

void NormalMap::bind(Shader* shader, unsigned int id) const
{
	if(id > 31)
	{
		tz::util::log::error("FrameBuffer bind ID ", id, " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	shader->set_uniform<int>("normal_map_sampler", id);
}

ParallaxMap::ParallaxMap(std::string filename): Texture(filename, false){}

void ParallaxMap::bind(Shader* shader, unsigned int id) const
{
	if(id > 31)
	{
		tz::util::log::error("FrameBuffer bind ID ", id, " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	shader->set_uniform<int>("parallax_map_sampler", id);
}

DisplacementMap::DisplacementMap(std::string filename): Texture(filename, false){}

void DisplacementMap::bind(Shader* shader, unsigned int id) const
{
	if(id > 31)
	{
		tz::util::log::error("FrameBuffer bind ID ", id, " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	shader->set_uniform<int>("displacement_map_sampler", id);
}

CubeMap::CubeMap(std::string right_texture, std::string left_texture, std::string top_texture, std::string bottom_texture, std::string back_texture, std::string front_texture): right_texture(std::move(right_texture)), left_texture(std::move(left_texture)), top_texture(std::move(top_texture)), bottom_texture(std::move(bottom_texture)), back_texture(std::move(back_texture)), front_texture(std::move(front_texture))
{
	glGenTextures(1, &(this->texture_handle));
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->texture_handle);
	std::vector<unsigned char*> face_data = this->load_textures();
	for(GLuint i = 0; i < face_data.size(); i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, this->width[static_cast<unsigned int>(i)], this->height[static_cast<unsigned int>(i)], 0, GL_RGBA, GL_UNSIGNED_BYTE, face_data[i]);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	for(auto& data : face_data)
		stbi_image_free(data);
}

CubeMap::CubeMap(std::string texture_directory, std::string skybox_name, std::string skybox_image_file_extension): CubeMap(texture_directory + skybox_name + "_rt" + skybox_image_file_extension, texture_directory + skybox_name + "_lf" + skybox_image_file_extension, texture_directory + skybox_name + "_up" + skybox_image_file_extension, texture_directory + skybox_name + "_dn" + skybox_image_file_extension, texture_directory + skybox_name + "_bk" + skybox_image_file_extension, texture_directory + skybox_name + "_ft" + skybox_image_file_extension){}

CubeMap::CubeMap(const CubeMap& copy): CubeMap(copy.right_texture, copy.left_texture, copy.top_texture, copy.bottom_texture, copy.back_texture, copy.front_texture){}

CubeMap::CubeMap(CubeMap&& move): texture_handle(move.texture_handle), right_texture(move.right_texture), left_texture(move.left_texture), top_texture(move.top_texture), bottom_texture(move.bottom_texture), back_texture(move.back_texture), front_texture(move.front_texture)
{
	for(unsigned int i = 0; i < 6; i++)
	{
		this->width[i] = move.width[i];
		this->height[i] = move.height[i];
		this->components[i] = move.components[i];
	}
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
		tz::util::log::error("FrameBuffer bind ID ", id, " is invalid. Must be between 1-31");
		return;
	}
	// this sets which texture we want to bind (id can be from 0 to 31)
	// GLTEXTURE0 is actually a number, so we can add the id instead of a massive switch statement
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->texture_handle);
	shader->set_uniform<int>("cube_map_sampler", id);
}

std::vector<unsigned char*> CubeMap::load_textures()
{
	std::vector<unsigned char*> image_data;
	image_data.reserve(6);
	image_data.push_back(stbi_load((this->right_texture).c_str(), &(this->width[0]), &(this->height[0]), &(this->components[0]), 4));
	image_data.push_back(stbi_load((this->left_texture).c_str(), &(this->width[1]), &(this->height[1]), &(this->components[1]), 4));
	image_data.push_back(stbi_load((this->top_texture).c_str(), &(this->width[2]), &(this->height[2]), &(this->components[2]), 4));
	image_data.push_back(stbi_load((this->bottom_texture).c_str(), &(this->width[3]), &(this->height[3]), &(this->components[3]), 4));
	image_data.push_back(stbi_load((this->back_texture).c_str(), &(this->width[4]), &(this->height[4]), &(this->components[4]), 4));
	image_data.push_back(stbi_load((this->front_texture).c_str(), &(this->width[5]), &(this->height[5]), &(this->components[5]), 4));
	return image_data;
}

RenderBuffer::RenderBuffer(int width, int height, GLenum internal_format): width(width), height(height), internal_format(internal_format), renderbuffer_handle(0)
{
	glGenRenderbuffers(1, &(this->renderbuffer_handle));
	glBindRenderbuffer(GL_RENDERBUFFER, this->renderbuffer_handle);
	glRenderbufferStorage(GL_RENDERBUFFER, this->internal_format, this->width, this->height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::~RenderBuffer()
{
	// still silently ignores 0's.
	glDeleteRenderbuffers(1, &(this->renderbuffer_handle));
}

FrameBuffer::FrameBuffer(int width, int height): width(width), height(height), framebuffer_handle(0), attachments({})
{
	glGenFramebuffers(1, &(this->framebuffer_handle));
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &(this->framebuffer_handle));
}

const std::unordered_map<GLenum, std::variant<Texture, RenderBuffer>>& FrameBuffer::get_attachments() const
{
	return this->attachments;
}

bool FrameBuffer::valid() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_handle);
	bool valid = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return valid;
}

bool FrameBuffer::has_colour() const
{
	return this->get_attachments().find(GL_COLOR_ATTACHMENT0) != this->get_attachments().end();
}

bool FrameBuffer::has_depth() const
{
	return this->get_attachments().find(GL_DEPTH_ATTACHMENT) != this->get_attachments().end();
}

bool FrameBuffer::has_stencil() const
{
	return this->get_attachments().find(GL_STENCIL_ATTACHMENT) != this->get_attachments().end();
}

void FrameBuffer::set_render_target()
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer_handle);
	glViewport(0, 0, this->width, this->height);
}