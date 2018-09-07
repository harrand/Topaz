#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include "graphics/graphics.hpp"
#include "graphics/shader.hpp"
#include <memory>
#include <optional>
#include <variant>

namespace tz::graphics
{
	/**
	* Minimum of implementation and 32. This is because if hardware allows 64 attachments, OpenGL headers currently dont even specify 32+ attachments (it goes to GL_DEPTH_ATTACHMENT). For this reason, it is the minimum of the two, for a fair compromise.
	*/
	constexpr int maximum_framebuffer_attachments = std::min(GL_MAX_COLOR_ATTACHMENTS, 32);

	/// Contains constants and helper functions about Assets used, such as textures and normal-maps.
	namespace asset
	{
		constexpr float default_parallax_map_scale = 0.04f;
		constexpr float default_parallax_map_offset = -0.5f;
		constexpr float default_displacement_factor = 0.25f;
		/**
		 * Unbind any existing Textures.
		 */
        inline void unbind_texture();
		/**
		 * Unbind any existing normal-maps, and notify the render-shader that a normal-map isn't being used.
		 * @param render_shader - The render-shader to notify of this change
		 */
        inline void unbind_normal_map(Shader& render_shader);
		/**
		 * Unbind any existing parallax-maps, and notify the render-shader that a parallax-map isn't being used.
		 * @param render_shader - The render-shader to notify of this change
		 */
        inline void unbind_parallax_map(Shader& render_shader);
		/**
		 * Unbind any existing displacement-maps, and notify the render-shader that a displacement-map isn't being used.
		 * @param render_shader - The render-shader to notify of this change
		 */
        inline void unbind_displacement_map(Shader& render_shader);
		/**
		 * Unbind any existing textures, normal-maps, parallax-maps and displacement-maps and notify the render-shader that none of them are being used.
		 * @param render_shader - The render-shader to notify of these changes
		 */
        inline void unbind_all_textures(Shader& render_shader);
	}

	/// What kind of data should the Texture's bitmap store? e.g a depth-texture.
	enum class TextureComponent : unsigned int
	{
		COLOUR_TEXTURE,
		HDR_COLOUR_TEXTURE,
		DEPTH_TEXTURE
	};

	/// What is the texture? e.g a normal-map.
	enum class TextureType : unsigned int
	{
		TEXTURE,
		NORMAL_MAP,
		PARALLAX_MAP,
		DISPLACEMENT_MAP,
	};

	/// What type of mipmapping should be used? e.g nearest.
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
* Bind Textures so that Topaz Meshes do not render monochromatically.
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
	 * Creates a completely empty colour-texture, but would be ready to be written to, if bound to a framebuffer.
	 * @param width - Width of the Texture.
	 * @param height - Width of the Texture.
	 */
	Texture(int width, int height, tz::graphics::TextureComponent component = tz::graphics::TextureComponent::COLOUR_TEXTURE, bool gamma_corrected = false);
	/**
	 * Loads a texture from a file.
	 * @param filename - Path to the image file.
	 * @param mipmapping - Whether mipmapping should be applied to this Texture.
	 * @param gamma_corrected - Whether gamma-correction should be applied to this Texture.
	 * @param store_bitmap - Whether the internal image-data should be stored in a local Bitmap.
	 */
	Texture(std::string filename, bool mipmapping = true, bool gamma_corrected = true);
    /**
     * Loads a texture from existing Pixel Data
     * @tparam Pixel - The type of Pixel to store in the Bitmap.
     * @param pixel_data - The bitmap from which to create a new Texture.
     */
    template<class Pixel>
    Texture(Bitmap<Pixel> pixel_data);
	/**
	 * Loads a texture from a font, given text.
	 * @param font - The Font containing the desired glyphs.
	 * @param text - String representing the text to be rendered into a texture.
	 * @param foreground_colour - Colour of the text.
	 */
	Texture(const Font& font, const std::string& text, SDL_Color foreground_colour);
	/**
	 * Copy a Texture by filename. If the texture was not loaded from an external file, perform image-data copy instead.
	 * @param copy - The Texture which data to copy from.
	 */
	Texture(const Texture& copy);
	/**
	 * Construct a Texture by occupying the underlying texture-handle of the moved parameter.
	 * @param move - Texture whos OpenGL handle should be taken.
	 */
	Texture(Texture&& move);
	/**
	 * Safely dispose of texture data.
	 */
	~Texture();
	/**
	 * Copy-assignment from an existing Texture
	 * @param rhs - The copy to assign this Texture to be.
	 * @return - This modified Texture.
	 */
	Texture& operator=(Texture rhs);
	/**
	 * Move-assignment from an existing Texture.
	 * @param rhs - The Texture whos OpenGL handle should be taken.
	 * @return - This modified Texture.
	 */
	Texture& operator=(Texture&& rhs);
	/**
	 * Bind this texture to the specified shader, with the uniform sampler-id specified.
	 * @param shader - Shader with which to render this Texture.
	 * @param id - Sampler-ID for the Shader to use.
	 */
	virtual void bind(Shader* shader, unsigned int id, const std::string& sampler_name = "texture_sampler") const;
	/**
	 * Get the width of the Texture.
	 * @return - The width of this Texture, in pixels.
	 */
	int get_width() const;
	/**
	 * Get the height of the Texture.
	 * @return - The height of this Texture, in pixels.
	 */
	int get_height() const;
	/**
	 * Get the mipmap-type used during construction of this Texture.
	 * @return - The mipmap-type used during construction. If none was used, returns tz::graphics::MipmapType::NONE
	 */
	tz::graphics::MipmapType get_mipmap_type() const;
	/**
	 * Query whether a mipmap was specified during Texture construction.
	 * @return - True if a mipmap was created. False otherwise
	 */
	bool has_mipmap() const;
	/**
	 * Get the local Bitmap created during Texture construction.
	 * @return - The Bitmap created during construction. If no Bitmap was created, returns an empty Bitmap
	 */
	const Bitmap<PixelRGBA>& get_bitmap() const;
	/**
	 * This is a normal Texture.
	 * @return - TextureType::TEXTURE
	 */
	virtual tz::graphics::TextureType get_texture_type() const{return tz::graphics::TextureType::TEXTURE;}
	tz::graphics::TextureComponent get_texture_component() const;
	/**
	 * Equate this Texture with another.
	 * @param rhs - The other Texture to equate with
	 * @return - True if the Textures are equal. False otherwise
	 */
	bool operator==(const Texture& rhs) const;
	friend class FrameBuffer;
protected:
	/**
	 * Retrieve the image data of the external image file specified by this->filename.
	 */
	unsigned char* load_texture(const char* file_name);
	/**
	 * Destroy the image data loaded previously from Texture::load_texture().
	 * @param imgdata - The existing image data to be destroyed
	 */
	void delete_texture(unsigned char* imgdata);
	/**
	 * Bind this Texture using the specified Shader and sampler-ID, but with additional specification for the desired name of the Shader's sampler.
	 * @param shader - The Shader with which to bind this Texture.
	 * @param id - Sampler-ID to represent this Texture.
	 * @param sampler_uniform_name - Name of the sampler variable in the Shader which should represent this Texture.
	 */
	void bind_with_string(Shader* shader, unsigned int id, const std::string& sampler_uniform_name) const;

	/// Underlying OpenGL texture-handle.
	GLuint texture_handle;
	/// Width of the texture, in pixels.
	int width;
	/// Height of the texture, in pixels.
	int height;
	/// Which components formulate the image data.
	int components;
	/// Stores the component type stored in this Texture (e.g a normal colour-texture or a depth-texture)
	tz::graphics::TextureComponent texture_component;
	/// Stores whether gamma-correction was specified for this texture.
	bool gamma_corrected;
	/// Optional storage for a local Bitmap of this texture's image data.
	Bitmap<PixelRGBA> bitmap;
private:
	/**
	 * Do not use.
	 * @param width - N/A
	 * @param height - N/A
	 * @param initialise_handle - N/A
	 */
	Texture(int width, int height, bool initialise_handle, tz::graphics::TextureComponent texture_component = tz::graphics::TextureComponent::COLOUR_TEXTURE, bool gamma_corrected = false);
	static void swap(Texture& lhs, Texture& rhs);
};

/**
 * Representation of a normal-map. It's like a texture, but each pixel represents a normal vector for a texel, not colour.
 */
class NormalMap: public Texture
{
public:
	/**
	 * Load a normal-map from an existing normal-map image file.
	 * @param filename - Path to the existing normal-map image file
	 */
	NormalMap(std::string filename);
	/**
	 * Bind this normal-map to a specified Shader.
	 * @param shader - The Shader to be bound with.
	 * @param id - Sampler-ID of this normal-map.
	 */
    virtual void bind(Shader* shader, unsigned int id, const std::string& sampler_name = "normal_map_sampler") const override;
	/**
	 * This is a normal-map Texture.
	 * @return TextureType::NORMAL_MAP
	 */
    virtual tz::graphics::TextureType get_texture_type() const override{return tz::graphics::TextureType::NORMAL_MAP;}
};

/**
 * Representation of a parallax-map. It's like a texture, but each pixel represents a texture-coordinate displacement scalar for a texel, not colour.
 */
class ParallaxMap: public Texture
{
public:
	/**
	 * Load a parallax-map from an existing parallax-map image file.
	 * @param filename - Path to the existing parallax-map image file
	 */
	ParallaxMap(std::string filename, float multiplier = tz::graphics::asset::default_parallax_map_scale, float offset = tz::graphics::asset::default_parallax_map_offset);
	/**
	* Bind this normal-map to a specified Shader.
	* @param shader - The Shader to be bound with.
	* @param id - Sampler-ID of this normal-map.
	*/
    virtual void bind(Shader* shader, unsigned int id, const std::string& sampler_name = "parallax_map_sampler") const override;
	/**
	 * This is a parallax-map Texture.
	 * @return - TextureType::PARALLAX_MAP
	 */
    virtual tz::graphics::TextureType get_texture_type() const override{return tz::graphics::TextureType::PARALLAX_MAP;}
private:
	float multiplier, bias;
};

/**
 * Representation of a displacement-map. It's like a texture, but each pixel represents an actual displacement vector for a texel, not colour.
 */
class DisplacementMap: public Texture
{
public:
	/**
	 * Load a displacement-map from an existing displacement-map image file.
	 * @param filename - Path to the existing displacement-map image file
	 */
	DisplacementMap(std::string filename, float displacement_factor = tz::graphics::asset::default_displacement_factor);
	DisplacementMap(Bitmap<PixelDepth> height_map, float displacement_factor = tz::graphics::asset::default_displacement_factor);
	/**
	* Bind this normal-map to a specified Shader.
	* @param shader - The Shader to be bound with.
	* @param id - Sampler-ID of this normal-map.
	*/
    virtual void bind(Shader* shader, unsigned int id, const std::string& sampler_name = "displacement_map_sampler") const override;
	/**
	 * This is a displacement-map Texture.
	 * @return - TextureType::DISPLACEMENT_MAP
	 */
    virtual tz::graphics::TextureType get_texture_type() const override{return tz::graphics::TextureType::DISPLACEMENT_MAP;}
private:
	float displacement_factor;
};

/**
* Used to construct skyboxes. Requires six textures; for each face of the skybox cube mesh.
*/
class CubeMap
{
public:
	/**
	 * Construct a CubeMap from six separate external image files.
	 * @param right_texture - The right face of the CubeMap
	 * @param left_texture - The left face of the CubeMap
	 * @param top_texture - The top face of the CubeMap
	 * @param bottom_texture - The bottom face of the CubeMap
	 * @param back_texture - The back face of the CubeMap
	 * @param front_texture - The front face of the CubeMap
	 */
	CubeMap(std::string right_texture, std::string left_texture, std::string top_texture, std::string bottom_texture, std::string back_texture, std::string front_texture);
	/**
	 * Construct a CubeMap from six separate external image files, using standard notation:
	 * Right-texture = "texture_directory/skybox_name_rt + skybox_image_file_extension"
	 * etc...
	 * @param texture_directory - Path to the directory containing all six image files.
	 * @param skybox_name - Name of the skybox for each image file.
	 * @param skybox_image_file_extension - Image type extension. Should be ".png" for PNG files, for example
	 */
	CubeMap(std::string texture_directory = "./", std::string skybox_name = "skybox", std::string skybox_image_file_extension = ".png");
	/**
	 * Copy from an existing CubeMap via the external image files.
	 * @param copy - The existing CubeMap to be copied from.
	 */
	CubeMap(const CubeMap& copy);
	/**
	 * Move from an existing CubeMap via the external image files.
	 * @param move - The existing CubeMap to be moved from.
	 */
	CubeMap(CubeMap&& move);
	/**
	 * Dispose of all image data safely.
	 */
	~CubeMap();
	/**
	 * Existing CubeMaps cannot be assigned.
	 * @param rhs - N/A
	 * @return - N/A
	 */
	CubeMap& operator=(const CubeMap& rhs) = delete;

	/**
	 * Bind this CubeMap using the specified Shader and sampler-ID.
	 * @param shader - The Shader to be bound with
	 * @param id - The Sampler-ID to be used
	 */
	void bind(Shader* shader, unsigned int id) const;
private:
	/**
	 * Load all of the six texture-faces from their external image files.
	 * @return - Vector of size-six of all the texture-faces' image data.
	 */
	std::vector<unsigned char*> load_textures();
	/// Underlying OpenGL cube-map handle.
	GLuint texture_handle;
	/// Texture-face external image file name.
	const std::string right_texture, left_texture, top_texture, bottom_texture, back_texture, front_texture;
	/// There are six-textures per CubeMap.
	static constexpr std::size_t number_of_textures = 6;
	/// Width of each texture-face, in pixels.
	int width[number_of_textures];
	/// Height of each texture-face, in pixels.
	int height[number_of_textures];
	/// Component-data of each texture-face.
	int components[number_of_textures];
};

namespace tz::graphics
{
    namespace height_map
    {
        template<typename Engine = std::default_random_engine, typename EngineResultType = std::default_random_engine::result_type>
        DisplacementMap generate_random_noise(int width, int height, float displacement_factor, Random<Engine, EngineResultType> noise_function = {});
        DisplacementMap generate_smooth_noise(int width, int height, float displacement_factor, SmoothNoise noise_function = {Random{}.next_int()});
        DisplacementMap generate_cosine_noise(int width, int height, float displacement_factor, float smoothness = tz::utility::numeric::default_smoothness, CosineNoise noise_function = Random{}.next_int());
    }
}

#include "texture.inl"
#endif