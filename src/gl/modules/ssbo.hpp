#ifndef TOPAZ_GL_MODULE_SSBO_HPP
#define TOPAZ_GL_MODULE_SSBO_HPP
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
     * Detects the "#ssbo <name>" directive in GLSL source.
     * Will replace the directive with valid GLSL syntax declaring an SSBO (using an auto-generated binding id.)
     * This generated binding ID can be obtained via this->get_buffer_id.
     */
    class SSBOModule : public ObjectAwareModule
    {
    public:
        /**
         * Construct an SSBOModule, creating SSBOs in the given Object when required.
         * @param o tz::gl::Object, will be the parent of any SSBOs that this module creates.
         */
        SSBOModule(tz::gl::Object* o);
        /**
         * Invoke the module, performing source transformation on the given string and creating SSBOs as required inside of the stored Object.
         * 
         * Note: Expect the stored SSBOs to be in-order as they appear when the source-code is read top-to-bottom.
         * @param source GLSL source-code to preprocess. Note that it doesn't have to be valid code.
         */
        virtual void operator()(std::string& source) const override;
        /**
         * Obtain the number of SSBO directives processed.
         * 
         * Note: This will always be empty before the module is invoked for the first time.
         * @return Number of SSBO directives processed.
         */
        std::size_t size() const;
        /**
         * Get the name of the SSBO at the given index.
         * 
         * Precondition: The index must be in-range (idx < this->size()).
         * @param idx Index to query. The index must be less than this->size().
         * @return String representing the name of the SSBO.
         */
        const std::string& get_name(std::size_t idx) const;
        /**
         * Get the buffer id handle of the SSBO at the given index.
         * 
         * Precondition: The index must be in-range (idx < this->size()).
         * @param idx Index to query. The index must be less than this->size().
         * @return Handle representing the ID of the SSBO in the stored tz::gl::Object.
         */
        std::size_t get_buffer_id(std::size_t idx) const;
    private:
        mutable std::vector<std::pair<std::string, std::size_t>> ssbo_name_id;
    };

    /**
     * @}
     */
    /**
     * @}
     */
}

#endif //TOPAZ_GL_MODULE_SSBO_HPP