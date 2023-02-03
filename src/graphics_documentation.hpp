/**
* @page graphics Graphics
* Topaz has its own high-level graphics API. The documentation for this API can be found within the @ref tz_gl2 section of the C++ API.
*
* The API is comprised of multiple layers.
* @image html res/tzgl.png
* @section layer_tzgl tz::gl
* The uppermost layer of the graphics API is what you will be interacting with as an application/game developer. This is all housed directly within the `tz::gl` namespace. There are various high-level concepts within `tz::gl` which you will need to understand to use the graphics library effectively.
* @subsubsection layer_tzgl_device Device
* There is a global @ref tz::gl::device object which is provided by the graphics library. You can retrieve a reference to this device at any time via @ref tz::gl::get_device(). Whenever the usage of a @ref tz::gl::device is mentioned, it is always this global device object.
* @subsubsection layer_tzgl_shaders Shaders
* To draw anything, you must have written a shader. Shaders are programs that run on the GPU. This means that they are unlike a normal program you would run via a CPU.
*
* Shaders are written in a variety of specialised programming languages. In Topaz, all shaders are written in the Topaz Shader Language, or TZSL for short. For more information on TZSL, read through the the @ref shaders section.
* @subsubsection layer_tzgl_renderers Renderers
* Renderers represent a single render-pass. A render-pass is a single execution of the rendering pipeline. In other words, it represents a single 'instance' of GPU work.
*
* To create a renderer, you need to specify the following (within a @ref tz::gl::renderer_info):
* - @ref layer_tzgl_shaders, *exactly one* of the following:
*   - A single compute shader.
*   - *At least* a vertex shader and a fragment shader.
*	  - Optionally, tessellation control and evaluation shaders.
* - Optionally, one or more @ref layer_tzgl_resources.
* - Optionally, @ref layer_tzgl_outputs. If no output is specified, the renderer will draw directly to the window.
*
* Once you have filled your @ref tz::gl::renderer_info, you can invoke @ref tz::gl::device::create_renderer() to create a new @ref tz::gl::renderer.
* @subsubsection layer_tzgl_resources Resources
* Resources are optional blocks of data that are usually passed to shaders. Resources are either buffers or images.
* - @ref tz::gl::buffer_resource
* - @ref tz::gl::image_resource
*
* Once you've created a resource, you can attach it to a renderer via @ref tz::gl::renderer_info::add_resource(). A resource is always owned by *exactly one* renderer, although you can also use @ref tz::gl::renderer_info::ref_resource() to have a renderer use a resource that is owned by another.
* @note If a renderer is using a resource owned by another renderer via `ref_resource`, it is essential that the lifetime of the owning renderer does not end before or during the user's `render()` invocation.
* @subsubsection layer_tzgl_outputs Outputs
* just output stuff.
*/
