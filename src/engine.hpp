#ifndef ENGINE_HPP
#define ENGINE_HPP
#include "window.hpp"
#include "command.hpp"
#include "timekeeper.hpp"
#include "player.hpp"

class Engine
{
public:
	Engine(Player& player, Window& wnd, std::string properties_path = "properties.mdl", unsigned int initial_fps = 60);
	~Engine(){};
	
	void update(std::size_t shader_index);
	
	const TimeKeeper& getTimeKeeper() const;
	const TimeProfiler& getTimeProfiler() const;
	
	const MDLF& getProperties() const;
	const MDLF& getResources() const;
	
	const Player& getPlayer() const;
	const Window& getWindow() const;
	const World& getWorld() const;
	const Shader& getDefaultShader() const;
	
	const std::vector<std::unique_ptr<Mesh>>& getMeshes() const;
	const std::vector<std::unique_ptr<Texture>>& getTextures() const;
	const std::vector<std::unique_ptr<NormalMap>>& getNormalMaps() const;
	const std::vector<std::unique_ptr<ParallaxMap>>& getParallaxMaps() const;
	const std::vector<std::unique_ptr<DisplacementMap>>& getDisplacementMaps() const;
	
	const Shader& getShader(std::size_t index) const;
	unsigned int getFPS() const;
	
	World& getWorldR();
private:
	TimeKeeper keeper;
	TimeProfiler profiler;
	const MDLF properties;
	const MDLF resources;
	const Shader default_shader;
	Player& player;
	Window& wnd;
	World world;
	std::vector<std::unique_ptr<Mesh>> meshes;
	std::vector<std::unique_ptr<Texture>> textures;
	std::vector<std::unique_ptr<NormalMap>> normal_maps;
	std::vector<std::unique_ptr<ParallaxMap>> parallax_maps;
	std::vector<std::unique_ptr<DisplacementMap>> displacement_maps;
	std::vector<Shader> extra_shaders;
	unsigned int fps;
};

#endif