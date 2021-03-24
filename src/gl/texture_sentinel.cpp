#include "gl/texture_sentinel.hpp"
#include "core/debug/assert.hpp"
#include <algorithm>

namespace tz::gl
{
	void TextureSentinel::register_handle([[maybe_unused]] GLuint64 handle)
	{
		#if TOPAZ_DEBUG
			topaz_hard_assert(!this->registered(handle), "tz::gl::TextureSentinel::register_handle(handle): Handle already has been registered!");
			this->declared.push_back(handle);
		#endif
	}

	void TextureSentinel::register_handle(GLuint tex_name, GLuint64 handle)
	{
		#if TOPAZ_DEBUG
			register_handle(handle);
			declared_tex_names[handle] = tex_name;
		#endif
	}

	void TextureSentinel::make_resident([[maybe_unused]] GLuint64 handle)
	{
		#if TOPAZ_DEBUG
			topaz_hard_assert(this->registered(handle), "tz::gl::TextureSentinel::make_resident(handle): Handle has not yet been registered!");
			topaz_hard_assert(!this->resident(handle), "tz::gl::TextureSentinel::make_resident(handle): Handle is already marked as resident!");
			this->residents.push_back(handle);
		#endif
	}

	std::size_t TextureSentinel::registered_handle_count() const
	{
		#if TOPAZ_DEBUG
			return this->residents.size();
		#else
			return 0;
		#endif
	}

	GLuint64 TextureSentinel::get_handle([[maybe_unused]] std::size_t id) const
	{
		#if TOPAZ_DEBUG
			topaz_assert(id < this->residents.size(), "tz::gl::TextureSentinel::get_handle(", id, "): Id out of range. Size = ", this->residents.size());
			return this->residents[id];
		#else
			return -1;
		#endif
	}

	bool TextureSentinel::registered([[maybe_unused]] GLuint64 handle) const
	{
		#if TOPAZ_DEBUG
			return std::find(this->declared.begin(), this->declared.end(), handle) != this->declared.end();
		#else
			return false;
		#endif
	}

	bool TextureSentinel::resident([[maybe_unused]] GLuint64 handle) const
	{
		#if TOPAZ_DEBUG
			return std::find(this->residents.begin(), this->residents.end(), handle) != this->residents.end();
		#else
			return false;
		#endif
	}

	bool TextureSentinel::ready(GLuint64 handle) const
	{
		return this->registered(handle) && this->resident(handle);
	}

	void TextureSentinel::notify_usage([[maybe_unused]] GLuint64 handle) const
	{
		#if TOPAZ_DEBUG
			topaz_hard_assert(this->ready(handle), "tz::gl::TextureSentinel::notify_usage(handle): Bindless handle is not ready!");
		#endif
	}

	std::optional<GLuint> TextureSentinel::get_texture_name(GLuint64 handle) const
	{
		#if TOPAZ_DEBUG
		if(this->declared_tex_names.find(handle) != this->declared_tex_names.end())
		{
			return {this->declared_tex_names.at(handle)};
		}
		#endif
		return {std::nullopt};
	}
}