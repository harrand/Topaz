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

        enum class DepthTesting
        {
            NeverPass,
            PassIfLess,
            PassIfEqual,
            PassIfLequal,
            PassIfGreater,
            PassIfNequal,
            PassIfGequal,
            AlwaysPass
        };

        void enable_wireframe_mode(bool wireframe) const;
		CullTarget get_culling() const;
		void set_culling(CullTarget culling = CullTarget::BackFaces) const;
        DepthTesting get_depth_testing() const;
        void set_depth_testing(DepthTesting depth_testing = DepthTesting::PassIfLess);
    };
}

#endif // TOPAZ_CORE_SETTINGS_HPP