#ifndef ENGINE_HPP
#define ENGINE_HPP
#include "gui_display.hpp"
#include "command.hpp"
#include "time.hpp"
#include "audio.hpp"

namespace tz
{
	void initialise();
	void terminate();
	constexpr char default_properties_path[] = "properties.mdl";
}

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
	const World& get_world() const;
	void set_world(World world);
	void add_to_world(Object object);
	void add_to_world(Entity entity);
	void add_to_world(EntityObject entity_object);
	void add_to_world(Light light, Shader& shader);
	void remove_from_world(Object object);
	void remove_from_world(Entity entity);
	void remove_from_world(EntityObject entity_object);
	void remove_from_world(Light light);
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
	Shader default_shader, default_gui_shader;
	Window* wnd;
	World world;
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