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

Engine::Engine(Window* wnd, std::string properties_path, unsigned int initial_fps, unsigned int tps): camera(Camera()), properties(RawFile(properties_path)), resources(RawFile(this->properties.get_tag("resources"))), default_shader(this->properties.get_tag("default_shader")), default_gui_shader(this->properties.get_tag("default_gui_shader")), wnd(wnd), world(this->properties.get_tag("default_world"), this->properties.get_tag("resources")), fps(initial_fps), tps(tps), update_command_executor(), tick_command_executor(), update_due(false)
{
	// move the camera to the world's spawn point.
	this->camera.set_position(this->world.get_spawn_point());
	// fill all the asset buffers via tz data manager
	tz::data::Manager(this->properties.get_tag("resources")).retrieve_all_data(this->meshes, this->textures, this->normal_maps, this->parallax_maps, this->displacement_maps);
	//test
	this->default_shader.add_uniform(Uniform(this->default_shader.get_program_handle(), "variadic_uniform_test", true));
	// read the properties file for any extra shaders specified (gui shader not included in this)
	for(std::string shader_path : this->properties.get_sequence("extra_shaders"))
		this->extra_shaders.emplace_back(shader_path);
}

void Engine::update(std::size_t shader_index)
{
	static Timer ticker;
	if(this->keeper.millis_passed(1000))
	{
		// update fps every second instead of every frame; suppresses random spikes in performance and reduces runtime overhead slightly
		this->fps = this->profiler.get_fps();
		this->profiler.reset();
		this->keeper.reload();
	}
	this->wnd->set_render_target();
	this->profiler.begin_frame();
	
	this->keeper.update();
	ticker.update();
	this->wnd->clear(0.0f, 0.0f, 0.0f, 1.0f);
	this->profiler.end_frame();
	
	this->world.render(this->camera, this->get_shader(shader_index), this->wnd->get_width(), this->wnd->get_height(), this->meshes, this->textures, this->normal_maps, this->parallax_maps, this->displacement_maps);
	
	for(auto command : this->update_command_executor.get_commands())
		command->operator()({});
	
	if(ticker.millis_passed(1000.0f/this->tps))
	{
		// update physics engine when the average time of a fixed 'tick' has passed
		for(auto tick_command : this->tick_command_executor.get_commands())
			tick_command->operator()({});
		this->world.update(this->tps);
		ticker.reload();
		this->update_due = true;
		
		//test
		bool test = this->default_shader.get_uniform_value<bool>("variadic_uniform_test");
		this->default_shader.set_uniform<bool>("variadic_uniform_test", !test);
	}
	else
		this->update_due = false;
	this->wnd->update();
	
	GLenum error;
		if((error = glGetError()) != GL_NO_ERROR)
			tz::util::log::error("OpenGL Error: ", error, "\n");
}

const Timer& Engine::get_timer() const
{
	return this->keeper;
}

const TimeProfiler& Engine::get_time_profiler() const
{
	return this->profiler;
}

const MDLF& Engine::get_properties() const
{
	return this->properties;
}

const MDLF& Engine::get_resources() const
{
	return this->resources;
}

const Window& Engine::get_window() const
{
	 return *(this->wnd);
}

const World& Engine::get_world() const
{
	return this->world;
}

void Engine::set_world(World world)
{
	this->world = world;
}

void Engine::add_to_world(Object object)
{
	this->world.add_object(object);
}

void Engine::add_to_world(EntityObject entity_object)
{
	this->world.add_entity_object(entity_object);
}

const Shader& Engine::get_default_shader() const
{
	return this->default_shader;
}

const Shader& Engine::get_default_gui_shader() const
{
	return this->default_gui_shader;
}

const std::vector<std::unique_ptr<Mesh>>& Engine::get_meshes() const
{
	return this->meshes;
}

const std::vector<std::unique_ptr<Texture>>& Engine::get_textures() const
{
	return this->textures;
}

const std::vector<std::unique_ptr<NormalMap>>& Engine::get_normal_maps() const
{
	return this->normal_maps;
}

const std::vector<std::unique_ptr<ParallaxMap>>& Engine::get_parallax_maps() const
{
	return this->parallax_maps;
}

const std::vector<std::unique_ptr<DisplacementMap>>& Engine::get_displacement_maps() const
{
	return this->displacement_maps;
}

const Shader& Engine::get_shader(std::size_t index) const
{
	if(index > this->extra_shaders.size())
		tz::util::log::error("Could not retrieve shader index ", index, ", retrieving default instead.");
	else if(index != 0)
		return this->extra_shaders[index - 1];
	return this->get_default_shader();
}

unsigned int Engine::get_fps() const
{
	return this->fps;
}

unsigned int Engine::get_tps() const
{
	return this->tps;
}

const CommandExecutor& Engine::get_update_command_executor() const
{
	return this->update_command_executor;
}

const CommandExecutor& Engine::get_tick_command_executor() const
{
	return this->tick_command_executor;
}

void Engine::add_update_command(Command* cmd)
{
	this->update_command_executor.register_command(cmd);
}

void Engine::remove_update_command(Command* cmd)
{
	this->update_command_executor.deregister_command(cmd);
}

void Engine::add_tick_command(Command* cmd)
{
	this->tick_command_executor.register_command(cmd);
}

void Engine::remove_tick_command(Command* cmd)
{
	this->tick_command_executor.deregister_command(cmd);
}

void Engine::register_listener(Listener& listener)
{
	this->wnd->register_listener(listener);
}

bool Engine::is_update_due() const
{
	return this->update_due;
}