//
// Created by Harrand on 12/04/2019.
//
#include "utility/render.hpp"

RenderableBoundingBox::RenderableBoundingBox(Transform transform, Asset asset, float wireframe_width): StaticObject(transform, asset), wire_width(wireframe_width){}

void RenderableBoundingBox::render(RenderPass render_pass) const
{
	glDisable(GL_CULL_FACE);
	tz::graphics::enable_wireframe_render(true, this->wire_width);
	StaticObject::render(render_pass);
	tz::graphics::enable_wireframe_render(false);
	glEnable(GL_CULL_FACE);
}

std::unique_ptr<Renderable> RenderableBoundingBox::unique_clone() const
{
	return std::make_unique<RenderableBoundingBox>(*this);
}

namespace tz::utility::render
{
	RenderableBoundingBox see_aabb(AssetBuffer& buffer, const AABB& box, const Vector3F& colour, float wire_width)
	{
		Mesh* cube_mesh = buffer.find_mesh("bounding_box");
		if(cube_mesh == nullptr)
			cube_mesh = &buffer.emplace_mesh("bounding_box", tz::graphics::create_cube());
		//PixelRGBA colour_pixel(static_cast<unsigned char>(colour.x * 255.0f), static_cast<unsigned char>(colour.y * 255.0f), static_cast<unsigned char>(colour.z * 255.0f));
		auto r = static_cast<unsigned char>(colour.x * 255.0f);
		auto g = static_cast<unsigned char>(colour.y * 255.0f);
		auto b = static_cast<unsigned char>(colour.z * 255.0f);
		unsigned char a = 255;
		Image colour_bitmap{{static_cast<std::byte>(r), static_cast<std::byte>(g), static_cast<std::byte>(b), static_cast<std::byte>(a)}, 1, 1};
		std::string texture_name = "colour-" + static_cast<std::string>(dynamic_cast<const Vector<3, float>&>(colour));
		Texture* colour_texture = buffer.find_texture(texture_name);
		if(colour_texture == nullptr)
			colour_texture = &buffer.emplace_texture(texture_name, colour_bitmap);
		// Definitely have a mesh now. Let's use it.
		return {Transform{{(box.get_minimum() + box.get_maximum()) / 2.0f}, {}, box.get_dimensions() / 2.0f}, Asset{cube_mesh, colour_texture}, wire_width};
	}

	tz::platform::OGLUniformBuffer pack_mvp(GLuint uniform_binding_id, Matrix4x4 model, Matrix4x4 view, Matrix4x4 projection)
	{
		tz::platform::OGLUniformBuffer ret{uniform_binding_id};
		std::vector<float> data;
		for(auto d : model.fill_data())
			data.push_back(d);
		for(auto d : view.fill_data())
			data.push_back(d);
		for(auto d : projection.fill_data())
			data.push_back(d);
		ret.insert(data, tz::platform::OGLBufferUsage{tz::platform::OGLBufferFrequency::STATIC, tz::platform::OGLBufferNature::DRAW});
		return ret;
	}
}