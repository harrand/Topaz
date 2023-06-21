#ifndef TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
#define TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"

// responsible for rendering meshes.
class mesh_renderer
{
public:
	mesh_renderer();
	tz::gl::renderer_handle get() const;
	void push_back_timeline() const;
	void dbgui();
private:
	// vertex buffer
	tz::gl::resource_handle vb = tz::nullhand;
	// index buffer
	tz::gl::resource_handle ib = tz::nullhand;
	// mesh meta buffer
	tz::gl::resource_handle mb = tz::nullhand;
	tz::gl::renderer_handle rh = tz::nullhand;
};


#endif // TZ_DEMO_MESH_DEMO_MESH_RENDERER_HPP
