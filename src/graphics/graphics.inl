#include "utility/log.hpp"

template<class Pixel>
Bitmap<Pixel>::Bitmap(std::vector<Pixel> pixels, std::size_t width, std::size_t height): pixels(pixels), width(width), height(height){}

template<class Pixel>
void Bitmap<Pixel>::resize(std::size_t width, std::size_t height)
{
	this->pixels.resize(width * height);
	this->width = width;
	this->height = height;
}

template<class Pixel>
const Pixel& Bitmap<Pixel>::get(const Vector2S& coordinate) const
{
	return this->pixels.at(coordinate.x + this->width * coordinate.y);
}

template<class Pixel>
void Bitmap<Pixel>::set(const Vector2S& coordinate, Pixel pixel)
{
	this->pixels.at(coordinate.x + this->width * coordinate.y) = pixel;
}

#ifndef topaz_assert
#define topaz_assert(x, y)
#endif

namespace tz::graphics
{
	inline void initialise()
	{
		topaz_assert(tz::graphics::has_context, "tz::graphics::initialise(): Error: Initialisation of tz::graphics aborted: No OpenGL context has been setup yet.");
		tz::debug::print("tz::graphics::initialise(): OpenGL context detected, initialising tz::graphics...\n");
		[[maybe_unused]] GLenum status = glewInit();
		topaz_assert(status == GLEW_OK, "tz::graphics::initialise(): Fatal Error: Initialisation of GLEW failed.\n\tInitialisation of tz::graphics unsuccessful!");
		tz::debug::print("tz::graphics::initialise(): Initialisation of GLEW successful.\n\tInitialised tz::graphics via GLEW (OpenGL).\n");
		[[maybe_unused]] int ttf_init_result = TTF_Init();
		topaz_assert(ttf_init_result == 0, "tz::graphics::initialise(): Fatal Error: Initialisation of SDL2_ttf failed.");
		tz::debug::print("tz::graphics::initialise(): Successfully initialised SDL2_ttf.\n");
		tz::graphics::initialised = true;
		glEnable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_MULTISAMPLE);
	}
	
	inline void terminate()
	{
		TTF_Quit();
		tz::debug::print("tz::graphics::terminate(): Terminated SDL2_ttf\n");
		tz::debug::print("tz::graphics::terminate(): Terminated tz::graphics via GLEW (OpenGL).\n");
	}

	inline void scene_render_mode()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_DEPTH_CLAMP);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	inline void gui_render_mode()
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_DEPTH_CLAMP);
		glDisable(GL_CULL_FACE);
	}

	inline void enable_wireframe_render(bool wireframes, float wireframe_width)
	{
		glPolygonMode(GL_FRONT_AND_BACK, wireframes ? GL_LINE : GL_FILL);
		if(wireframes)
			glLineWidth(wireframes ? wireframe_width : 1.0f);
	}
}