#ifndef ENGINE_HPP
#define ENGINE_HPP
#include "graphics/gui.hpp"
#include "command.hpp"
#include "time.hpp"
#include "audio/audio.hpp"
#include "engine_meta.hpp"

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
	 * Constructs an Engine with all specifications.
	 * @param window - Pointer to the Window to be bound to
	 * @param properties_path - Path to the Properties File
	 * @param tps - Desired number of application-ticks per second
	 */
	Engine(Window* window, std::string properties_path = tz::default_properties_path, unsigned int tps = 30);
	
	/**
	 * This should be invoked in your application's game loop.
	 * @param shader_index - The shader index to be used to decide which Shader to use. Defaults to zero, which selects the default_shader
	 */
	void update(std::size_t shader_index = 0);
	/**
	 * Read-only access to the time-profiler.
	 * @return - Underlying time-profiler
	 */
	const TimeProfiler& get_time_profiler() const;
	/**
	 * Read-only access to the EngineMeta.
	 * @return - Underlying engine-meta
	 */
	const EngineMeta& get_meta() const;
	/**
	 * Access the Window that this Engine is attached to.
	 * @return - Reference to the Window that the Engine is attached to
	 */
	const Window& get_window() const;
	/**
	 * Access list of Mesh assets.
	 * @return - Container of Mesh assets.
	 */
	const std::vector<std::unique_ptr<Mesh>>& get_meshes() const;
	/**
	 * Access list of Texture assets.
	 * @return - Container of Texture assets.
	 */
	const std::vector<std::unique_ptr<Texture>>& get_textures() const;
	/**
	 * Access list of NormalMap assets.
	 * @return - Container of NormalMap assets.
	 */
	const std::vector<std::unique_ptr<NormalMap>>& get_normal_maps() const;
	/**
	 * Access list of ParallaxMap assets.
	 * @return - Container of ParallaxMap assets.
	 */
	const std::vector<std::unique_ptr<ParallaxMap>>& get_parallax_maps() const;
	/**
	 * Access list of DisplacementMap assets.
	 * @return - Container of DisplacementMap assets.
	 */
	const std::vector<std::unique_ptr<DisplacementMap>>& get_displacement_maps() const;
	/**
	 * Get shader by index.
	 * @param index - Shader index to sub into. If index = 0 or is out of range of extra-shaders, will return the default shader. Otherwise, it shall return the extra shader at that index
	 * @return - Reference to the desired Shader
	 */
	Shader& get_shader(std::size_t index);
	/**
	 * Get the FPS.
	 * @return - Instantaneous fps
	 */
	unsigned int get_fps() const;
	/**
	 * Get the specified number of ticks per second back when the instance was constructed.
	 * @return - TPS
	 */
	unsigned int get_tps() const;
	/**
	 * Read-only access to the update CommandExecutor.
	 * @return - Reference to the update CommandExecutor
	 */
	const CommandExecutor& get_update_command_executor() const;
	/**
	 * Read-only access to the tick CommandExecutor.
	 * @return - Reference to the tick CommandExecutor
	 */
	const CommandExecutor& get_tick_command_executor() const;
	/**
	 * Add an existing Command to the update CommandExecutor.
	 * @param cmd - Existing Command to be added to the update CommandExecutor
	 */
	void add_update_command(Command* cmd);
	/**
	 * Construct in-place a trivial command into the update CommandExecutor.
	 * @tparam Functor - Type of the functor (should be deduced)
	 * @param functor - The functor value
	 * @return - Pointer to the constructed trivial command
	 */
    template<typename Functor>
    TrivialFunctor<Functor>* emplace_trivial_update_command(Functor&& functor);
    /**
     * Construct in-place a static command into the update CommandExecutor.
     * @tparam Functor - Type of the functor (should be deduced)
     * @tparam FunctorParameters - Types of the functor parameters (may require explicity)
     * @param functor - The functor value
     * @param parameters - The parameter values
     * @return - Pointer to the constructed static command
     */
    template<typename Functor, typename... FunctorParameters>
    StaticFunctor<Functor, FunctorParameters...>* emplace_static_update_command(Functor&& functor, FunctorParameters&&... parameters);
    /**
     * Remove an existing Command from the update CommandExecutor.
     * @param cmd - The existing Command to be removed
     */
	void remove_update_command(Command* cmd);
	/**
	 * Add an existing Command to the tick CommandExecutor.
	 * @param cmd - Existing Command to be added to the tick CommandExecutor
	 */
	void add_tick_command(Command* cmd);
	/**
	 * Construct in-place a trivial command into the tick CommandExecutor.
	 * @tparam Functor - Type of the functor (should be deduced)
	 * @param functor - The functor value
	 * @return - Pointer to the constructed trivial command
	 */
	template<typename Functor>
	TrivialFunctor<Functor>* emplace_trivial_tick_command(Functor&& functor);
	/**
     * Construct in-place a static command into the tick CommandExecutor.
     * @tparam Functor - Type of the functor (should be deduced)
     * @tparam FunctorParameters - Types of the functor parameters (may require explicity)
     * @param functor - The functor value
     * @param parameters - The parameter values
     * @return - Pointer to the constructed static command
     */
	template<typename Functor, typename... FunctorParameters>
	StaticFunctor<Functor, FunctorParameters...>* emplace_static_tick_command(Functor&& functor, FunctorParameters&&... parameters);
	/**
     * Remove an existing Command from the tick CommandExecutor.
     * @param cmd - The existing Command to be removed
     */
	void remove_tick_command(Command* cmd);
	/**
	 * Query whether an Engine update is expected to happen this tick.
	 * @return - True if an update is expected. False otherwise
	 */
	bool is_update_due() const;
	/// Public access to the Camera used to view this Engine's Scene.
	Camera camera;
	/// Public access to this Engine's Scene.
	Scene scene;
private:
	/// Engine Meta object.
	EngineMeta meta;
public:
	/// Public access to the default-shader and default-gui-shader.
	Shader default_shader, default_gui_shader;
private:
	/// The timers used in this Engine's scheduling.
	Timer seconds_timer, tick_timer;
	/// The profiler used to calculate FPS readings.
	TimeProfiler profiler;
	/// Pointer to the bound Window.
	Window* window;
	/// Container of Mesh assets.
	std::vector<std::unique_ptr<Mesh>> meshes;
	/// Container of Texture assets.
	std::vector<std::unique_ptr<Texture>> textures;
	/// Container of NormalMap assets.
	std::vector<std::unique_ptr<NormalMap>> normal_maps;
	/// Container of ParallaxMap assets.
	std::vector<std::unique_ptr<ParallaxMap>> parallax_maps;
	/// Container of DisplacementMap assets.
	std::vector<std::unique_ptr<DisplacementMap>> displacement_maps;
	/// Container of all extra-shaders.
	std::vector<Shader> extra_shaders;
	/// FPS cache.
	unsigned int fps;
	/// Read-only TPS specification.
	const unsigned int tps;
	/// Engine underlying CommandExecutors.
	CommandExecutor update_command_executor, tick_command_executor;
	/// Stores whether an Engine update is due this tick.
	bool update_due;
public:
	/// Default Texture asset.
	const Texture default_texture;
	/// Default NormalMap asset.
	const NormalMap default_normal_map;
	/// Default ParallaxMap asset.
	const ParallaxMap default_parallax_map;
	/// Default DisplacementMap asset.
	const DisplacementMap default_displacement_map;
};

#include "engine.inl"

#endif