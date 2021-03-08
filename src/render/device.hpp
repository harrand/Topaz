#ifndef TOPAZ_RENDER_DEVICE_HPP
#define TOPAZ_RENDER_DEVICE_HPP
#include "gl/index_snippet.hpp"
#include <initializer_list>
#include <optional>
#include <vector>

// Forward declares
namespace tz
{
	namespace gl
	{
		class IFrame;
		class ShaderProgram;
		class Object;
	}
}

namespace tz::render
{
	/**
	 * \addtogroup tz_render Topaz Rendering Library (tz::render)
	 * High-level interface for 3D and 2D hardware-accelerated graphics programming. Used in combination with the \ref tz_gl "Topaz Graphics Library".
	 * @{
	 */

	/**
	 * Represents and controls the conditions under which render-invocations will take place.
	 * 
	 * Devices will always render to the given frame, using the given program and drawing via the given object.
	 * Attempting to render when any of these are invalid will assert and invoke UB.
	 */
	class Device
	{
	public:
		/**
		 * Construct a Device with the given conditions.
		 * @param frame Frame-object to render into.
		 * @param prg Shader Program to bind and render with.
		 * @param object Object whose data will be used to render.
		 */
		Device(tz::gl::IFrame* frame = nullptr, tz::gl::ShaderProgram* prg = nullptr, tz::gl::Object* object = nullptr);
		/**
		 * Set the Devices currently-used frame. The old value will no longer be used.
		 * Note: If the frame is incomplete (see tz::gl::IFrame::complete()) or nullptr is passed, the Device will not be ready (see tz::render::Device::ready()).
		 * @param frame Instance of the frame interface to render into in subsequent render passes.
		 */
		void set_frame(tz::gl::IFrame* frame);
		/**
		 * Sets the Devices currently-used shader program. The old value will no longer be used.
		 * Note: If the program is not usable (see tz::gl::ShaderProgram::usable()) or nullptr is passed, the Device will not be ready (see tz::render::Device::ready()).
		 * @param prg Shader Program object to use in subsequent render passes.
		 */
		void set_program(tz::gl::ShaderProgram* prg);
		/**
		 * Sets the Devices currently-used graphics object. The old value will no longer be used.
		 * Note: This will clear the currently registered IBO handle. This does not effect readiness but know that tz::render::Device::render() will early-out if no IBO id is registered so you will have to register a new one.
		 * @param object Object from which to source buffer data in subsequent render passes.
		 */
		void set_object(tz::gl::Object* object);
		/**
		 * Query as to whether this Device has currently registered the given IBuffer.
		 * @param buffer Buffer to query.
		 * @return True if the resource buffer has previously been registered (see tz::render::Device::add_resource_buffer(IBuffer*)).
		 */
		bool contains_resource_buffer(const tz::gl::IBuffer* buffer) const;
		/**
		 * Register the given resource buffer to this Device, causing it to be bound in subsequent render-passes.
		 * Note: If the buffer has already been registered, nothing interesting happens.
		 * Precondition: The IBuffer is a resource-buffer (either a UBO or SSBO). Otherwise, the next render invocation will invoke UB without asserting.
		 * @param buffer Buffer to register to this Device.
		 */
		void add_resource_buffer(const tz::gl::IBuffer* buffer);
		void add_resource_buffers(std::initializer_list<const tz::gl::IBuffer*> buffers);
		/**
		 * Unregister the given resource buffer from the Device, preventing it from being bound in subsequent render-passes.
		 * Note: If the buffer has not previously been registered, nothing interesting happens.
		 * @param buffer Buffer to unregister from this Device.
		 */
		void remove_resource_buffer(const tz::gl::IBuffer* buffer);
		/**
		 * Query as to whether the Device expects to render patches or triangles.
		 * @return True if patches are expected, otherwise triangles.
		 */
		bool is_patches() const;
		/**
		 * Inform the Device to render patches, or the default primitive (triangles).
		 * @param patches True if patches are to be rendered, false if triangles.
		 */
		void set_is_patches(bool patches);
		/**
		 * Retrieve the null-device. This device is invalid and should never be used to render.
		 * @return The null-device.
		 */
		static Device null_device();
		/**
		 * Query as to whether this device is equal to the null device.
		 * @return True if this device is the null-device. Otherwise false
		 */
		bool is_null() const;
		/**
		 * Refer subsequent render-invocations to the given ID handle corresponding to an index-buffer inside of the Object.
		 * Note: This will clear the list of indices as they may no longer be valid.
		 * Precondition: ibo_id must be a valid index in the Object and refer to a valid index-buffer. Otherwise, this will assert and invoke UB.
		 * @param ibo_id Handle ID corresponding to the index-buffer which will be used in render invocations.
		 */
		void set_handle(std::size_t ibo_id);
		/**
		 * Refer subsequent render-invocations to the given ID handle corresponding to an index-buffer inside of a different Object.
		 * Note: This will clear the list of indices as they may no longer be valid.
		 * Precondition: ibo_id must be a valid index in the Object and refer to a valid index-buffer. Otherwise, this will assert and invoke UB.
		 * @param ibo_id Handle ID corresponding to the index-buffer which will be used in render invocations.
		 */
		void set_handle(tz::gl::Object* object, std::size_t ibo_id);
		/**
		 * Provide a list of snippets of offsets into the index-buffer.
		 * 
		 * Note: After providing a snippet, future render-invocations will invoke multi-render.
		 * Precondition: snippet must contain index-ranges valid corresponding to the index-buffer which will be bound against this Device.
		 * @param snippet Snippet containing index-ranges used in MDI.
		 */
		void set_indices(tz::gl::IndexSnippetList indices);
		/**
		 * Invoke a render-invocation, making the Object emit a draw-call via the ibo_id set via this->set_handle.
		 * Note: All registered resource-buffers will be bound directly before rendering the Object (see tz::render::Device::add_resource_buffer(IBuffer*)).
		 * Note: If an index-snippet was passed via this->set_snippet, then the Object will emit a multi-render call instead using the MDI draw command list generated by the snippet.
		 * Precondition: An ID handle must have been set via this->set_handle. Otherwise, this method will early-out and do nothing.
		 * Precondition: If a snippet was provided by this->set_snippet, it must contain index-ranges valid in the context of the ibo_id provided by this->set_handle earlier. Otherwise, this will assert and invoke UB.
		 */
		void render() const;
		/**
		 * Force the attached Frame to clear its backbuffer. The Frame will be bound prior; no need to do it yourself.
		 * Precondition: The attached frame is not nullptr. Otherwise this will assert and invoke UB.
		 */
		void clear() const;
		/**
		 * Query as to whether the Device is ready to be used for rendering.
		 * A Device is ready if all of the following is true:
		 * - References a valid and complete IFrame (see tz::gl::IFrame::complete()).
		 * - References a valid and useable ShaderProgram (see tz::gl::ShaderProgram::usable()).
		 * - References a valid tz::gl::Object (Only check so far is nullness so a false-positive is possible).
		 * Note: A Device must be ready before it is rendered.
		 * @return True if ready. Otherwise false.
		 */
		bool ready() const;
		/// Deep-comparison
		bool operator==(const Device& rhs) const;
	private:
		void ensure_bound() const;
		/**
		 * Ensures that all indices specified by all snippets exist within the IBO.
		 */
		static bool sanity_check(const tz::gl::IndexSnippetList& indices, const tz::gl::IBO& ibo);

		tz::gl::IFrame* frame;
		tz::gl::ShaderProgram* program;
		tz::gl::Object* object;
		std::optional<std::size_t> ibo_id;
		tz::gl::IndexSnippetList snippets;
		std::vector<const tz::gl::IBuffer*> resource_buffers;
		bool patches;
	};

	/**
	 * @}
	 */
}

#endif // TOPAZ_RENDER_DEVICE_HPP