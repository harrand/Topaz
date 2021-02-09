#ifndef TOPAZ_DUI_MODULE_GL_TAB_HPP
#define TOPAZ_DUI_MODULE_GL_TAB_HPP

namespace tz::gl
{
    class Object;
}

namespace tz::dui::gl
{
    void draw_tab();
    void track_object(tz::gl::Object* object);
}

#endif // TOPAZ_DUI_MODULE_GL_TAB_HPP