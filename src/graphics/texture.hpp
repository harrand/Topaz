#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include <memory>
#include "graphics.hpp"
#include "shader.hpp"
#include <optional>
#include <variant>

namespace tz::graphics
{
	constexpr unsigned int frame_buffer_default_size = 256;
	constexpr unsigned int frame_buffer_default_width = frame_buffer_default_size;
	constexpr unsigned int frame_buffer_default_height = frame_buffer_default_size;
	constexpr unsigned int depth_texture_default_size = 1024;
	constexpr unsigned int depth_texture_default_width = depth_texture_default_size;
	constexpr unsigned int depth_texture_default_height = depth_texture_default_size;
	
	/**
	* Minimum of implementation and 32. This is because if hardware allows 64 attachments, OpenGL headers currently dont even specify 32+ attachments (it goes to GL_DEPTH_ATTACHMENT). For this reason, it is the minimum of the two, for a fair compromise.
	*/
	constexpr int maximum_framebuffer_attachments = std::min(GL_MAX_COLOR_ATTACHMENTS, 32);

	constexpr Vector4<unsigned char> default_texture_colour = Vector4<unsigned char>(std::array<unsigned char, 4>({255, 0, 255, 255}));
    constexpr Vector4<unsigned char> default_normal_map_colour = Vector4<unsigned char>(std::array<unsigned char, 4>({128, 128, 255, 255}));
    constexpr Vector4<unsigned char> default_parallax_map_colour = Vector4<unsigned char>(std::array<unsigned char, 4>({128, 128, 128, 255}));
    constexpr Vector4<unsigned char> default_displacement_map_colour = Vector4<unsigned char>(std::array<unsigned char, 4>({0, 0, 0, 255}));

	constexpr PixelRGBA default_texture_pixel = PixelRGBA(255, 0, 255, 255);
    constexpr PixelRGBA default_normal_map_pixel = PixelRGBA(128, 128, 255, 255);
    constexpr PixelRGBA default_parallax_map_pixel = PixelRGBA(128, 128, 128, 255);
    constexpr PixelRGBA default_displacement_map_pixel = PixelRGBA(0, 0, 0, 255);

	enum class TextureType : unsigned int
	{
		TEXTURE,
		NORMAL_MAP,
		PARALLAX_MAP,
		DISPLACEMENT_MAP,
		TEXTURE_TYPES,
	};
	
	enum class MipmapType : GLint
	{
		NEAREST = GL_NEAREST_MIPMAP_NEAREST,
		LINEAR = GL_LINEAR_MIPMAP_NEAREST,
		NEAREST_MULTIPLE = GL_NEAREST_MIPMAP_LINEAR,
		LINEAR_MULTIPLE = GL_LINEAR_MIPMAP_LINEAR,
		NONE,
	};
}

/**
* Holds pixel and colour data and can interact with OpenGL buffers.
* Bind Textures so that Topaz Meshes do not render monochromoatically.
*/
class Texture
{
public:
	/**
	* Creates an uninitialised texture.
	* This allocates a texture-handle but nothing else, so is not ready for rendering.
	*/
	Texture();
	/**
	* Creates a completely empty texture, but would be ready to be written to, if bound to a framebuffer.
	*/
	Texture(int width, int height);
	/**
	* Loads a texture from a file.
	*/
	Texture(std::string filename, bool mipmapping = true, bool gamma_corrected = true, bool store_bitmap = false);
    /**
     * Loads a texture from existing Pixel Data
     */
    template<class Pixel>
    Texture(Bitmap<Pixel> pixel_data);
	/**
	* Loads a texture from a font, given text.
	*/
	Texture(const Font& font, const std::string& text, SDL_Color foreground_colour, bool store_bitmap = false);
	Texture(const Texture& copy);
	Texture(Texture&& move);
	~Texture();
	Texture& operator=(Texture&& rhs);
	
	virtual void bind(Shader* shader, unsigned int id) const;
	bool has_file_name() const;
	const std::string& get_file_name() const;
	int get_width() const;
	int get_height() const;
	bool has_bitmap() const;
	tz::graphics::MipmapType get_mipmap_type() const;
	bool has_mipmap() const;
	Bitmap<PixelRGBA> get_bitmap() const;
	virtual tz::graphics::TextureType get_texture_type() const{return tz::graphics::TextureType::TEXTURE;}
	template<class T>
	static T* get_from_link(const std::string& texture_link, const std::vector<std::unique_ptr<T>>& all_textures);
	
	bool operator==(const Texture& rhs) const;
	friend class FrameBuffer;
protected:
	unsigned char* load_texture();
	void delete_texture(unsigned char* imgdata);
	void bind_with_string(Shader* shader, unsigned int id, const std::string& sampler_uniform_name) const;
	
	std::optional<std::string> filename;
	GLuint texture_handle;
	int width, height, components;
	bool gamma_corrected;
	std::optional<Bitmap<PixelRGBA>> bitmap;
private:
	Texture(int width, int height, bool initialise_handle);
};

/**
 * Representation of a normal-map. It's like a texture, but each pixel represents a normal vector for a texel, not colour.
 */
class NormalMap: public Texture
{
public:
	NormalMap(std::string filename): Texture(filename, false, false, false){};
	NormalMap(): Texture(Bitmap<PixelRGBA>({tz::graphics::default_normal_map_pixel}, 1, 1)){}
    virtual void bind(Shader* shader, unsigned int id) const override{this->bind_with_string(shader, id, "normal_map_sampler");}
	virtual tz::graphics::TextureType get_texture_type() const override{return tz::graphics::TextureType::NORMAL_MAP;}
};

/**
 * Representation of a parallax-map. It's like a texture, but each pixel represents a texture-coordinate displacement scalar for a texel, not colour.
 */
class ParallaxMap: public Texture
{
public:
	ParallaxMap(std::string filename): Texture(filename, false, false, false){};
    ParallaxMap(): Texture(Bitmap<PixelRGBA>({tz::graphics::default_parallax_map_pixel}, 1, 1)){}
    virtual void bind(Shader* shader, unsigned int id) const override{this->bind_with_string(shader, id, "parallax_map_sampler");}
	virtual tz::graphics::TextureType get_texture_type() const override{return tz::graphics::TextureType::PARALLAX_MAP;}
};

/**
 * Representation of a displacement-map. It's like a texture, but each pixel represents an actual displacement vector for a texel, not colour.
 */
class DisplacementMap: public Texture
{
public:
	DisplacementMap(std::string filename): Texture(filename, false, false, false){};
    DisplacementMap(): Texture(Bitmap<PixelRGBA>({tz::graphics::default_displacement_map_pixel}, 1, 1)){}
    virtual void bind(Shader* shader, unsigned int id) const override{this->bind_with_string(shader, id, "displacement_map_sampler");}
	virtual tz::graphics::TextureType get_texture_type() const override{return tz::graphics::TextureType::DISPLACEMENT_MAP;}
};

/**
* Used to construct skyboxes. Requires six textures; for each face of the skybox cube mesh.
*/
class CubeMap
{
public:
	CubeMap(std::string right_texture, std::string left_texture, std::string top_texture, std::string bottom_texture, std::string back_texture, std::string front_texture);
	CubeMap(std::string texture_directory = "./", std::string skybox_name = "skybox", std::string skybox_image_file_extension = ".png");
	CubeMap(const CubeMap& copy);
	CubeMap(CubeMap&& move);
	~CubeMap();
	CubeMap& operator=(const CubeMap& rhs) = delete;
	
	void bind(Shader* shader, unsigned int id) const;
private:
	std::vector<unsigned char*> load_textures();
	GLuint texture_handle;
	const std::string right_texture, left_texture, top_texture, bottom_texture, back_texture, front_texture;
	static constexpr std::size_t number_of_textures = 6;
	int width[number_of_textures], height[number_of_textures], components[number_of_textures];
};

/**
* Simple wrapper for an OpenGL RenderBuffer. It's just a POD as they're write-only.
*/
class RenderBuffer
{
public:
	RenderBuffer(int width, int height, GLenum internal_format = GL_RGBA);
	/**
	* OpenGL RenderBuffers are write-only, so cannot possibly read the data in which to copy or move.
	*/
	RenderBuffer(const RenderBuffer& copy) = delete;
	RenderBuffer(RenderBuffer&& move);
	~RenderBuffer();
	/**
	* RenderBuffer::operator= shall act like a pointer-copy; both share the same handle. However, when one dies the other becomes invalid, so this will be deleted too.
	*/
	RenderBuffer& operator=(const RenderBuffer& rhs) = delete;
	friend class FrameBuffer;
private:
	const int width, height;
	const GLenum internal_format;
	GLuint renderbuffer_handle;
};

/**
* Something to draw to that isn't a window.
* FrameBuffer attachments can either be a Texture or a RenderBuffer.
*/
class FrameBuffer
{
public:
	FrameBuffer(int width, int height);
	~FrameBuffer();
	/**
	* Build an instance of either Texture or RenderBuffer in-place into the framebuffer.
	*/
	template<class Buffer, typename... Args>
	Buffer& emplace(GLenum attachment, Args&&... args);
	/**
	* Build an instance of Texture in-place into the framebuffer.
	*/
	template<typename... Args>
	Texture& emplace_texture(GLenum attachment, Args&&... args);
	/**
	* Build an instance of RenderBuffer in-place into the framebuffer.
	*/
	template<typename... Args>
	RenderBuffer& emplace_renderbuffer(GLenum attachment, Args&&... args);
	/**
	* Read-only access to all attachments to this framebuffer.
	*/
	const std::unordered_map<GLenum, std::variant<Texture, RenderBuffer>>& get_attachments() const;
	std::unordered_map<GLenum, std::reference_wrapper<const Texture>> get_texture_attachments() const;
	/**
	* Returns true if OpenGL sees the framebuffer as complete.
	*/
	bool valid() const;
	/**
	* Returns true if this framebuffer has a Texture or RenderBuffer with the attachment GL_COLOR_ATTACHMENTx, where x == attachment_index.
	*/
	bool has_colour(unsigned int attachment_index = 0) const;
	/**
	* Returns true if this framebuffer has a Texture or RenderBuffer with the attachment GL_DEPTH_ATTACHMENT.
	*/
	bool has_depth() const;
	/**
	* Returns true if this framebuffer has a Texture or RenderBuffer with the attachment GL_STENCIL_ATTACHMENT.
	*/
	bool has_stencil() const;
	/**
	* Make the output value of the fragment-shader write to an existing buffer with the corresponding attachment type (e.g specifying GL_COLOR_ATTACHMENT0 will write to the Texture/RenderBuffer applied to GL_COLOR_ATTACHMENT0).
	*/
	void set_output_attachment(GLenum attachment) const;
	/**
	* Perform an OpenGL clear operation on the framebuffer.
	*/
	void clear(GLbitfield mask = (GL_COLOR_BUFFER_BIT), float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f) const;
	/**
	* Bind and sets the viewpoint to this framebuffer.
	* This means that any render calls will apply to this framebuffer.
	*/
	void set_render_target() const;
private:
	int width, height;
	GLuint framebuffer_handle;
	std::unordered_map<GLenum, std::variant<Texture, RenderBuffer>> attachments;
};

namespace tz::graphics::texture
{
	extern std::shared_ptr<Texture> default_texture;
    extern std::shared_ptr<NormalMap> default_normal_map;
    extern std::shared_ptr<ParallaxMap> default_parallax_map;
    extern std::shared_ptr<DisplacementMap> default_displacement_map;
}

#include "texture.inl"
#endif