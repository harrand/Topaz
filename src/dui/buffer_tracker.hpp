#ifndef TOPAZ_GL_TZ_IMGUI_BUFFER_TRACKER_HPP
#define TOPAZ_GL_TZ_IMGUI_BUFFER_TRACKER_HPP
#include "dui/window.hpp"
#include "gl/buffer.hpp"
#include <optional>

// Forward declares
namespace tz::gl
{
	class Object;
}

namespace tz::dui::gl
{
	class BufferTracker : public tz::dui::DebugWindow
	{
	public:
		BufferTracker(tz::gl::Object* object);
		void track_buffer(std::size_t buffer_handle);
		void target_object(tz::gl::Object* object);
		virtual void render() override;
		void reset();
	private:
		tz::gl::Object* object;
		std::optional<std::size_t> tracked_buffer_id;
		int view_offset;
		int view_size;
	};
}

#endif // TOPAZ_GL_TZ_IMGUI_BUFFER_TRACKER_HPP