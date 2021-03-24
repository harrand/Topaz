#ifndef TOPAZ_GL_TEXTURE_SENTINEL_HPP
#define TOPAZ_GL_TEXTURE_SENTINEL_HPP
#include <vector>
#include <glad/glad.h>
#include <unordered_map>

namespace tz::gl
{
	/** 
	 * GL_ARB_bindless_texture is an incredibly dangerous extension, so the sentinel is designed to minimise hazards should the feature ever be misused.
	 * Note: GL_ARB_bindless_texture is required for this sentinel to be useful. It will not assert/invoke UB of any kind without it, but is not useful.
	 * Note: Terminal textures make use of GL_ARB_bindless_texture.
	 * Note: TextureSentinels are entirely stubbed out in release.
	*/
	class TextureSentinel
	{
	public:
		TextureSentinel() = default;
		/**
		 * Register an existing bindless texture handle.
		 * Note: This can be retrieved from a terminal texture via Texture::get_terminal_handle.
		 * Precondition: The given handle has not already been registered. Otherwise, this will hard-assert.
		 * @param handle Bindless texture handle to register.
		 */
		void register_handle(GLuint64 handle);
		/**
		 * Register an existing bindless texture handle aswell as its corresponding texture name.
		 * Note: The handle can be retrieved from a terminal texture via Texture::get_terminal_handle.
		 * Precondition: The given handle has not already been registered. Otherwise, thsi will hard-assert.
		 * @param tex_name Texture name to associate with the coming bindless handle.
		 * @param handle Bindless texture handle to register.
		 */
		void register_handle(GLuint tex_name, GLuint64 handle);
		/**
		 * Make a registered bindless texture handle resident, signifying that it can be used in subsequent draw calls.
		 * Precondition: The given handle has already been registered. Otherwise, this will hard-assert.
		 * Precondition: The given handle has not yet been made resident. Otherwise, this will hard-assert.
		 */
		void make_resident(GLuint64 handle);
		/**
		 * Retrieve the number of handles currently registered.
		 * Note: This includes resident and non-resident handles.
		 * @return Number of registered handles.
		 */
		std::size_t registered_handle_count() const;
		/**
		 * Retrieve a registered handle by its id.
		 * DEPRECATED: This makes no sense to use in its current form and will be reworked/removed. Do not use this method.
		 * Precondition: The given id is within range. Otherwise, this will assert and invoke UB.
		 * @param id The first handle to be registered will have id 0, the next 1, and so on... I know, this is terrible.
		 * @return Handle at the given id.
		 */
		GLuint64 get_handle(std::size_t id) const;
		/**
		 * Query as to whether the given handle has been registered before.
		 * @param handle Handle for which to query status.
		 * @return True if handle is registered. Otherwise false.
		 */
		bool registered(GLuint64 handle) const;
		/**
		 * Query as to whether the given handle is resident and thus is useable for rendering.
		 * Note: It is logical to deduce that any handle which is resident must also be registered, but this is not guaranteed. This may not be the case for example if this sentinel has been misused but hard-asserts were disabled so it hasn't crashed.
		 * Note: For the reason above, you should prefer this->ready().
		 * @param handle Handle for which to query status.
		 * @return True if the handle is resident. Otherwise false.
		 */
		bool resident(GLuint64 handle) const;
		/**
		 * Logically equivalent to this->resident(), but should be preferred.
		 * Query as to whether the given handle is both registered and resident.
		 * @param handle Handle for which to query status.
		 * @return True if the handle is resident. Otherwise false.
		 */
		bool ready(GLuint64 handle) const;
		/**
		 * Notify the sentinel that the given handle is about to be used. You should invoke this for all terminal textures that you're about to use to render with.
		 * Note: If handle is not ready, this will hard-assert. This should always be a check you make -- Misusing a bindless texture in this way can crash your GPU or bring down the whole OS. Much better to hard-assert.
		 * @param handle Handle for which to query status.
		 */
		void notify_usage(GLuint64 handle) const;
		std::optional<GLuint> get_texture_name(GLuint64 handle) const;
	private:
	#if TOPAZ_DEBUG
		std::vector<GLuint64> declared;
		std::vector<GLuint64> residents;
		std::unordered_map<GLuint64, GLuint> declared_tex_names;
	#endif
	};
}
#endif // TOPAZ_GL_TEXTURE_SENTINEL_HPP