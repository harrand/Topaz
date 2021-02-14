#ifndef TOPAZ_CORE_SETTINGS_HPP
#define TOPAZ_CORE_SETTINGS_HPP

namespace tz
{
    class RenderSettings
    {
    public:
        enum class CullTarget
        {
            BackFaces,
            FrontFaces,
            Both,
            Nothing
        };

        void enable_wireframe_mode(bool wireframe) const;
		CullTarget get_culling() const;
		void set_culling(CullTarget culling = CullTarget::BackFaces) const;
    };
}

#endif // TOPAZ_CORE_SETTINGS_HPP