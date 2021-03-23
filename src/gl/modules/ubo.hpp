#ifndef TOPAZ_GL_MODULE_UBO_HPP
#define TOPAZ_GL_MODULE_UBO_HPP
#include "gl/shader_preprocessor.hpp"
#include <vector>
#include <string>

namespace tz::gl::p
{
	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */
	/**
	 * \addtogroup tz_gl_p tz::gl Shader Preprocessing Module (TZGLP)
	 * @{
	 */

	/**
	 * Detects the "#ubo <name>" directive in GLSL source.
	 * Will replace the directive with valid GLSL syntax declaring an UBO (using an auto-generated binding id.)
	 * This generated binding ID can be obtained via this->get_buffer_id.
	 */
	class UBOModule : public ObjectAwareModule
	{
	public:
		/**
		 * Construct an UBOModule, creating UBOs in the given Object when required.
		 * @param o tz::gl::Object, will be the parent of any UBOs that this module creates.
		 */
		UBOModule(tz::gl::Object& o);
		/**
		 * Invoke the module, performing source transformation on the given string and creating UBOs as required inside of the stored Object.
		 * 
		 * Note: Expect the stored UBOs to be in-order as they appear when the source-code is read top-to-bottom.
		 * @param source GLSL source-code to preprocess. Note that it doesn't have to be valid code.
		 */
		virtual void operator()(std::string& source) const override;
		/**
		 * Obtain the number of UBO directives processed.
		 * 
		 * Note: This will always be empty before the module is invoked for the first time.
		 * @return Number of UBO directives processed.
		 */
		std::size_t size() const;
		/**
		 * Get the name of the UBO at the given index.
		 * 
		 * Precondition: The index must be in-range (idx < this->size()).
		 * @param idx Index to query. The index must be less than this->size().
		 * @return String representing the name of the UBO.
		 */
		const std::string& get_name(std::size_t idx) const;
		/**
		 * Get the buffer id handle of the UBO at the given index.
		 * 
		 * Precondition: The index must be in-range (idx < this->size()).
		 * @param idx Index to query. The index must be less than this->size().
		 * @return Handle representing the ID of the UBO in the stored tz::gl::Object.
		 */
		std::size_t get_buffer_id(std::size_t idx) const;
		std::size_t get_buffer_id(const std::string& ubo_name) const;
	private:
		std::optional<std::size_t> get_ubo_id_by_name(const std::string& ubo_name) const;
		mutable std::vector<std::pair<std::string, std::size_t>> ubo_name_id;
	};

	/**
	 * @}
	 */
	/**
	 * @}
	 */
}

#endif //TOPAZ_GL_MODULE_SSBO_HPP