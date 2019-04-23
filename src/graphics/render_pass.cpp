//
// Created by Harrand on 26/03/2019.
//

#include "render_pass.hpp"

RenderContext::RenderContext(Shader& object_shader, std::optional<std::reference_wrapper<Shader>> sprite_shader): object_shader(&object_shader), sprite_shader(nullptr)
{
	if(sprite_shader.has_value())
		this->sprite_shader = &sprite_shader.value().get();
}

const Shader& RenderContext::get_object_shader() const
{
	return *this->object_shader;
}

void RenderContext::set_object_shader(Shader& object_shader)
{
	this->object_shader = &object_shader;
}

const Shader* RenderContext::get_sprite_shader() const
{
	return this->sprite_shader;
}

void RenderContext::set_sprite_shader(Shader& sprite_shader)
{
	this->sprite_shader = &sprite_shader;
}

void RenderContext::disable_sprite_shader()
{
	this->sprite_shader = nullptr;
}

RenderPass::RenderPass(const Window& window, RenderContext context, const Camera& camera): window(&window), context(context), camera(&camera){}

const Window& RenderPass::get_window() const
{
	return *this->window;
}

void RenderPass::set_window(const Window& window)
{
	this->window = &window;
}

const RenderContext& RenderPass::get_render_context() const
{
	return this->context;
}

void RenderPass::set_render_context(RenderContext context)
{
	this->context = context;
}

const Camera& RenderPass::get_camera() const
{
	return *this->camera;
}

void RenderPass::set_camera(const Camera& camera)
{
	this->camera = &camera;
}