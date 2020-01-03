#ifndef TOPAZ_RENDER_DEVICE_HPP
#define TOPAZ_RENDER_DEVICE_HPP
#include <vector>
#include <initializer_list>

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
		void specify_handles(std::initializer_list<std::size_t> ids);
		void render() const;
		void clear() const;
		void reset();
	private:
		void ensure_bound() const;

		tz::gl::IFrame* frame;
		tz::gl::ShaderProgram* program;
		tz::gl::Object* object;
		std::vector<std::size_t> index_ids;
	};
}

#endif // TOPAZ_RENDER_DEVICE_HPP