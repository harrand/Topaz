#include "engine.hpp"
#include "graphics.hpp"
#include "data.hpp"

void tz::initialise()
{
	tz::util::log::message("Initialising Topaz...");
	SDL_Init(SDL_INIT_EVERYTHING);
	tz::util::log::message("Initialised SDL2.");
	tz::audio::initialise();
	tz::util::log::message("Initialised Topaz. Ready to receive OpenGL context...");
}

void tz::terminate()
{
	tz::util::log::message("Terminating Topaz...");
	tz::graphics::terminate();
	tz::audio::terminate();
	SDL_Quit();
	tz::util::log::message("Terminated SDL2.");
	tz::util::log::message("Terminated Topaz.");
}

Engine::Engine(Camera& camera, Window& wnd, std::string properties_path, unsigned int initial_fps, unsigned int tps): properties(RawFile(properties_path)), resources(RawFile(this->properties.getTag("resources"))), default_shader(this->properties.getTag("default_shader")), default_gui_shader(this->properties.getTag("default_gui_shader")), camera(camera), wnd(wnd), world(this->properties.getTag("default_world"), this->properties.getTag("resources")), fps(initial_fps), tps(tps)
{
	this->camera.getPositionR() = this->world.getSpawnPoint();
	tz::util::log::message("Set player position to world spawn.");
	tz::util::log::message("Loading assets from '", this->properties.getTag("resources"), "'...");
	tz::util::log::message("Loaded ", tz::data::Manager(this->properties.getTag("resources")).retrieveAllData(this->meshes, this->textures, this->normal_maps, this->parallax_maps, this->displacement_maps), " assets.");
	for(std::string shaderPath : this->properties.getSequence("extra_shaders"))
		this->extra_shaders.push_back(Shader(shaderPath));
}

void Engine::update(std::size_t shader_index)
{
	static TimeKeeper ticker;
	if(this->keeper.millisPassed(1000))
	{
		this->fps = this->profiler.getFPS();
		this->profiler.reset();
		this->keeper.reload();
	}
	this->wnd.setRenderTarget();
	this->profiler.beginFrame();
	
	this->keeper.update();
	ticker.update();
	this->wnd.clear(0.0f, 0.0f, 0.0f, 1.0f);
	this->profiler.endFrame();
	
	this->world.render(this->camera, this->getShader(shader_index), this->wnd.getWidth(), this->wnd.getHeight(), this->meshes, this->textures, this->normal_maps, this->parallax_maps, this->displacement_maps);
	
	if(ticker.millisPassed(1000/this->tps))
	{
		this->world.update(this->tps);
		ticker.reload();
	}
	default_gui_shader.bind();
	Matrix4x4 i = Matrix4x4::identity();
	default_gui_shader.update(i.fillData(), i.fillData(), i.fillData());
	this->wnd.update();
	
	GLenum error;
		if((error = glGetError()) != GL_NO_ERROR)
			tz::util::log::error("OpenGL Error: ", error, "\n");
}

const TimeKeeper& Engine::getTimeKeeper() const
{
	return this->keeper;
}

const TimeProfiler& Engine::getTimeProfiler() const
{
	return this->profiler;
}

const MDLF& Engine::getProperties() const
{
	return this->properties;
}

const MDLF& Engine::getResources() const
{
	return this->resources;
}

const Camera& Engine::getCamera() const
{
	return this->camera;
}

const Window& Engine::getWindow() const
{
	 return this->wnd;
}

const World& Engine::getWorld() const
{
	return this->world;
}

const Shader& Engine::getDefaultShader() const
{
	return this->default_shader;
}

Camera& Engine::getCameraR()
{
	return this->camera;
}

Window& Engine::getWindowR()
{
	return this->wnd;
}

World& Engine::getWorldR()
{
	return this->world;
}

const std::vector<std::unique_ptr<Mesh>>& Engine::getMeshes() const
{
	return this->meshes;
}

const std::vector<std::unique_ptr<Texture>>& Engine::getTextures() const
{
	return this->textures;
}

const std::vector<std::unique_ptr<NormalMap>>& Engine::getNormalMaps() const
{
	return this->normal_maps;
}

const std::vector<std::unique_ptr<ParallaxMap>>& Engine::getParallaxMaps() const
{
	return this->parallax_maps;
}

const std::vector<std::unique_ptr<DisplacementMap>>& Engine::getDisplacementMaps() const
{
	return this->displacement_maps;
}

const Shader& Engine::getShader(std::size_t index) const
{
	if(index > this->extra_shaders.size())
		tz::util::log::error("Could not retrieve shader index ", index, ", retrieving default instead.");
	else if(index != 0)
		return this->extra_shaders.at(index - 1);
	return this->getDefaultShader();
}

unsigned int Engine::getFPS() const
{
	return this->fps;
}

unsigned int Engine::getTPS() const
{
	return this->tps;
}