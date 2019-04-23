//
// Created by Harrand on 26/03/2019.
//

#ifndef TOPAZ_RENDER_PASS_HPP
#define TOPAZ_RENDER_PASS_HPP
#include "core/window.hpp"

/**
 * Provides a graphical context to a scene when issuing draw calls.
 * This structure tells the scene with which shaders to issue draw calls with.
 * Each RenderContext must have exactly one object shader, and optionally a second shader for sprite rendering.
 * Remark: If a sprite-shader is not provided, topaz will NOT attempt to use the object shader to render the sprites. Instead, rendering the sprites shall be skipped for this draw call. If one does not wish to render sprites, simply omit the sprite shader in the given RenderContext.
 */
class RenderContext
{
public:
	RenderContext(Shader& object_shader, std::optional<std::reference_wrapper<Shader>> sprite_shader = std::nullopt);
	const Shader& get_object_shader() const;
	void set_object_shader(Shader& object_shader);
	const Shader* get_sprite_shader() const;
	void set_sprite_shader(Shader& sprite_shader);
	void disable_sprite_shader();
	friend class StaticObject;
	friend class Sprite;
	friend class InstancedStaticObject;
	friend class InstancedDynamicObject;
	friend class Scene;
private:
	Shader* object_shader;
	Shader* sprite_shader;
};

class RenderPass
{
public:
	RenderPass(const Window& window, RenderContext context, const Camera& camera);
	const Window& get_window() const;
	void set_window(const Window& window);
	const RenderContext& get_render_context() const;
	void set_render_context(RenderContext context);
	const Camera& get_camera() const;
	void set_camera(const Camera& camera);
private:
	const Window* window;
	RenderContext context;
	const Camera* camera;
};


#endif //TOPAZ_RENDER_PASS_HPP
