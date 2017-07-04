#include "engine.hpp"

Engine::Engine(Player& player, Window& wnd, std::string properties_path, unsigned int initial_fps): properties(RawFile(properties_path)), resources(RawFile(this->properties.getTag("resources"))), default_shader(this->properties.getTag("default_shader")), player(player), wnd(wnd), world(this->properties.getTag("default_world"), this->properties.getTag("resources")), fps(initial_fps)
{
	this->player.setPosition(this->world.getSpawnPoint());
	LogUtility::message("Set player position to world spawn.");
	LogUtility::message("Loading assets from '", this->properties.getTag("resources"), "'...");
	LogUtility::message("Loaded ", DataTranslation(this->properties.getTag("resources")).retrieveAllData(this->meshes, this->textures, this->normalMaps, this->parallaxMaps, this->displacementMaps), " assets.");
	this->world.addEntity(this->player);
	for(std::string shaderPath : this->properties.getSequence("extra_shaders"))
		this->extraShaders.push_back(Shader(shaderPath));
}

void Engine::update(std::size_t shader_index)
{
	if(this->keeper.millisPassed(1000))
	{
		this->fps = this->profiler.getFPS();
		this->profiler.reset();
		this->keeper.reload();
	}
	this->wnd.setRenderTarget();
	this->profiler.beginFrame();
	
	this->keeper.update();
	this->wnd.clear(0.0f, 0.0f, 0.0f, 1.0f);
	this->profiler.endFrame();
	
	this->world.update(this->fps, this->player.getCamera(), this->getShader(shader_index), this->wnd.getWidth(), this->wnd.getHeight(), this->meshes, this->textures, this->normalMaps, this->parallaxMaps, this->displacementMaps);
	this->wnd.update();
	
	GLenum error;
		if((error = glGetError()) != GL_NO_ERROR)
			LogUtility::error("OpenGL Error: ", error, "\n");
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

const Player& Engine::getPlayer() const
{
	return this->player;
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
	return this->normalMaps;
}

const std::vector<std::unique_ptr<ParallaxMap>>& Engine::getParallaxMaps() const
{
	return this->parallaxMaps;
}

const std::vector<std::unique_ptr<DisplacementMap>>& Engine::getDisplacementMaps() const
{
	return this->displacementMaps;
}

const Shader& Engine::getShader(std::size_t index) const
{
	if(index > this->extraShaders.size())
		LogUtility::error("Could not retrieve shader index ", index, ", retrieving default instead.");
	else if(index != 0)
		return this->extraShaders.at(index - 1);
	return this->getDefaultShader();
}

unsigned int Engine::getFPS() const
{
	return this->fps;
}

World& Engine::getWorldR()
{
	return this->world;
}