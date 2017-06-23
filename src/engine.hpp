#ifndef ENGINE_HPP
#define ENGINE_HPP
#include "listeners.hpp"
#include "timekeeper.hpp"
#include "player.hpp"

class Engine
{
public:
	Engine(Player& player, Window& wnd, std::string properties_path = RES_POINT + "/properties.mdl");
	~Engine();
	void update(std::size_t shader_index, MouseController& mc, KeybindController& kc);
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
	
	World& getWorldR();
private:
	TimeKeeper keeper;
	TimeProfiler profiler;
	const MDLF properties;
	const MDLF resources;
	const Shader default_shader;
	Player& player;
	Window& wnd;
	unsigned int secondsLifetime;
	World world;
	std::vector<std::unique_ptr<Mesh>> meshes;
	std::vector<std::unique_ptr<Texture>> textures;
	std::vector<std::unique_ptr<NormalMap>> normalMaps;
	std::vector<std::unique_ptr<ParallaxMap>> parallaxMaps;
	std::vector<std::unique_ptr<DisplacementMap>> displacementMaps;
	std::vector<Shader> extraShaders;
	unsigned int fps;
};

#endif