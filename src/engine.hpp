#ifndef ENGINE_HPP
#define ENGINE_HPP
#include "graphics/gui.hpp"
#include "command.hpp"
#include "time.hpp"
#include "audio/audio.hpp"

namespace tz
{
	/**
	* Invoke this to initialise all Topaz modules and be able to use all features.
	* Note that a Topaz window must have been constructed at least once before the graphics module fully initialises.
	*/
	void initialise();
	/** Invoke this to close all Topaz modules and free corresponding memory to prevent droplets
	* Once this function is executed, most features will cease to work properly and will likely invoke undefined behaviour.
	*/
	void terminate();
	constexpr char default_properties_path[] = "properties.mdl";
}

/**
* Hulking class holding pretty much everything you'll need to use Topaz.
* Due to its verbosity, it is only recommended to use this class for hobbyist/non-commercial purposes. Using this essentially provides stabilisers and handholding to using Topaz.
*/
class Engine
{
public:
	/**
	* Constructs the Engine. Should be invoked after tz::initialise and Window construction.
	* window = Address of the Topaz window to render into.
	* properties_path = The absolute path to the Topaz properties file (normally called properties.mdl)
	* tps = Number of tick updates per second (this affects runtime of physics etc, not rendering). Default is 30, although you can use less or more, depending on how precise you need physics etc to run at.
	*/
	Engine(Window* window, std::string properties_path = tz::default_properties_path, unsigned int tps = 30);
	Engine(const Engine& copy) = default;
	Engine(Engine&& move) = default;
	Engine& operator=(const Engine& rhs) = default;
	~Engine() = default;
	
	/**
	* Invoke this in your main application loop.
	* For clarification of 'shader_index', see documentation for Engine::get_shader(std::size_t).
	*/
	void update(std::size_t shader_index);
	const TimeProfiler& get_time_profiler() const;
	/**
	* Read/Edit the properties file.
	*/
	const MDLF& get_properties() const;
	/**
	* Read/Edit the resources file.
	*/
	const MDLF& get_resources() const;
	/**
	* Access the window that the Engine instance currently is hooked to.
	*/
	const Window& get_window() const;
	/**
	* Access lists of all assets.
	*/
	const std::vector<std::unique_ptr<Mesh>>& get_meshes() const;
	const std::vector<std::unique_ptr<Texture>>& get_textures() const;
	const std::vector<std::unique_ptr<NormalMap>>& get_normal_maps() const;
	const std::vector<std::unique_ptr<ParallaxMap>>& get_parallax_maps() const;
	const std::vector<std::unique_ptr<DisplacementMap>>& get_displacement_maps() const;
	/**
	* Get shader by index. If index = 1, will return the default shader.
	*/
	Shader& get_shader(std::size_t index);
	/**
	* Get instantaneous fps
	*/
	unsigned int get_fps() const;
	/**
	* Get the specified number of ticks per second back when the instance was constructed.
	*/
	unsigned int get_tps() const;
	/**
	* Read the update command executor (for rendering etc)
	*/
	const CommandExecutor& get_update_command_executor() const;
	/**
	* Read the tick command executor (for physics updates etc)
	*/
	const CommandExecutor& get_tick_command_executor() const;
	/**
	* Add a custom command to the update comand executor.
	*/
	void add_update_command(Command* cmd);
	/**
	* Remove a command from the update command executor.
	*/
	void remove_update_command(Command* cmd);
	/**
	* Add a custom command to the tick command executor.
	*/
	void add_tick_command(Command* cmd);
	/**
	* Remove a command from the tick command executor.
	*/
	void remove_tick_command(Command* cmd);
	/**
	* Register a listener to the Engine instance. Use this to easily integrate keyboard/mouse input the engine with your application.
	*/
	void register_listener(Listener& listener);
	/**
	* Returns true if a physics update will occur next update. Use-cases for this mainly include when you need to synchronise your own functionality with the physics updates (which you should really use add_tick_command(Command*) for.)
	*/
	bool is_update_due() const;
	/**
	* Editing fields of these public members is well-defined. But as far as Topaz is concerned, re-assigning them is unspecified behaviour.
	*/
	Camera camera;
	Scene scene;
private:
	MDLF properties, resources;
public:
	Shader default_shader, default_gui_shader;
private:
	Timer seconds_timer, tick_timer;
	TimeProfiler profiler;
	Window* window;
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