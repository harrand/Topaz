#ifndef TOPAZ_GL_SCREEN_HPP
#define TOPAZ_GL_SCREEN_HPP
namespace tz::gl
{
    /**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */

    /**
     * Represents a physical hardware monitor.
     */
    class Screen
    {
    public:
        /**
         * Retrieve the primary screen. This is usually the screen of the monitor where elements like the task bar or global menu bar are located.
         */
        static Screen primary();
        /**
         * Retrieve the width of the screen, in pixels.
         */
        int get_width() const;
        /**
         * Retrieve the height of the screen, in pixels.
         */
        int get_height() const;
        /**
         * Retrieve the aspect-ratio of the screen.
         */
        float get_aspect_ratio() const;
    private:
        Screen(int width, int height);

        int width;
        int height;
    };

    /**
     * @}
     */
}
#endif // TOPAZ_GL_SCREEN_HPP