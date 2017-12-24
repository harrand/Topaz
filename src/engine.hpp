#ifndef ENGINE_HPP
#define ENGINE_HPP
#include "gui.hpp"
#include "command.hpp"
#include "time.hpp"
#include "audio.hpp"

namespace tz
{
	// Invoke this to initialise all Topaz modules and be able to use all features. Note that a Topaz window must have been constructed at least once before the graphics module fully initialises.
	void initialise();
	// Invoke this to close all Topaz modules and free corresponding memory to prevent droplets. Once this function is executed, most features will cease to work properly and will likely invoke undefined behaviour.
	void terminate();
	constexpr char default_properties_path[] = "properties.mdl";
}

/*
	Hulking class holding pretty much everything you'll need to use Topaz. Due to its verbosity, it is only recommended to use this class for hobbyist/non-commercial purposes. Using this essentially provides stabilisers and handholding to using Topaz.
*/
class Engine
{
public:
	Engine(Window* wnd, std::string properties_path = tz::default_properties_path, unsigned int initial_fps = 60, unsigned int tps = 30);
	Engine(const Engine& copy) = default;
	Engine(Engine&& move) = default;
	Engine& operator=(const Engine& rhs) = default;
	~Engine() = default;
	
	void update(std::size_t shader_index);
	const TimeProfiler& get_time_profiler() const;
	const MDLF& get_properties() const;
	const MDLF& get_resources() const;
	const Window& get_window() const;
	const Scene& get_scene() const;
	void set_scene(Scene scene);
	void add_to_scene(Object object);
	void add_to_scene(Entity entity);
	void add_to_scene(EntityObject entity_object);
	void add_to_scene(Light light, Shader& shader);
	void remove_from_scene(Object object);
	void remove_from_scene(Entity entity);
	void remove_from_scene(EntityObject entity_object);
	void remove_from_scene(Light light);
	const Shader& get_default_shader() const;
	const Shader& get_default_gui_shader() const;
	const std::vector<std::unique_ptr<Mesh>>& get_meshes() const;
	const std::vector<std::unique_ptr<Texture>>& get_textures() const;
	const std::vector<std::unique_ptr<NormalMap>>& get_normal_maps() const;
	const std::vector<std::unique_ptr<ParallaxMap>>& get_parallax_maps() const;
	const std::vector<std::unique_ptr<DisplacementMap>>& get_displacement_maps() const;
	Shader& get_shader(std::size_t index);
	unsigned int get_fps() const;
	unsigned int get_tps() const;
	const CommandExecutor& get_update_command_executor() const;
	const CommandExecutor& get_tick_command_executor() const;
	void add_update_command(Command* cmd);
	void remove_update_command(Command* cmd);
	void add_tick_command(Command* cmd);
	void remove_tick_command(Command* cmd);
	void register_listener(Listener& listener);
	bool is_update_due() const;
	
	Camera camera;
private:
	Timer seconds_timer, tick_timer;
	TimeProfiler profiler;
	MDLF properties;
	MDLF resources;
public:
	Shader default_shader, default_gui_shader;
private:
	Window* wnd;
public:
	Scene scene;
private:
	std::vector<std::unique_ptr<Mesh>> meshes;
	std::vector<std::unique_ptr<Texture>> textures;
	std::vector<std::unique_ptr<NormalMap>> normal_maps;
	std::vector<std::unique_ptr<ParallaxMap>> parallax_maps;
	std::vector<std::unique_ptr<DisplacementMap>> displacement_maps;
	std::vector<Shader> extra_shaders;
	unsigned int fps;
	const unsigned int tps;
	CommandExecutor update_command_executor, tick_command_executor;
	bool update_due;
};

#endif