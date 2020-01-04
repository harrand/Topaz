#ifndef TOPAZ_RENDER_DEVICE_HPP
#define TOPAZ_RENDER_DEVICE_HPP
#include "render/index_snippet.hpp"
#include <initializer_list>
#include <optional>

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
	class Device
	{
	public:
		Device(tz::gl::IFrame* frame, tz::gl::ShaderProgram*, tz::gl::Object* object);
		void set_handle(std::size_t ibo_id);
		void set_snippet(tz::render::IndexSnippet snippet);
		void render() const;
		void clear() const;
		void reset();
	private:
		void ensure_bound() const;

		tz::gl::IFrame* frame;
		tz::gl::ShaderProgram* program;
		tz::gl::Object* object;
		std::optional<std::size_t> ibo_id;
		tz::render::IndexSnippet snippet;
	};
}

#endif // TOPAZ_RENDER_DEVICE_HPP