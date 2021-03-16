#include "phys/resolvers/position.hpp"
#include "phys/body.hpp"

namespace tz::phys
{
    void PositionResolver::solve_all(CollisionList& collisions, [[maybe_unused]] float delta_millis)
    {
        constexpr bool is_kinematic = true;
        using DeltaPair = std::pair<tz::Vec3, tz::Vec3>;
        std::vector<DeltaPair> deltas;
        // Collect each delta pair
        for(const CollisionSituation& situation : collisions)
        {
            float inv_mass_a = 1.0f / situation.a.mass;
            float inv_mass_b = 1.0f / situation.b.mass;

            constexpr float percent = 0.8f;
            constexpr float slop = 0.01f;
            float correction_coeff_numerator = percent * std::fmax(situation.point.depth() - slop, 0.0f);
            float correction_coeff_denominator = inv_mass_a + inv_mass_b;
            tz::Vec3 correction = situation.point.normal() * correction_coeff_numerator / correction_coeff_denominator;

            tz::Vec3 delta_a{0.0f, 0.0f, 0.0f};
            tz::Vec3 delta_b{0.0f, 0.0f, 0.0f};

            if(is_kinematic)
            {
                delta_a = correction * inv_mass_a * -1.0f;
                delta_b = correction * inv_mass_b;
            }
            deltas.emplace_back(delta_a, delta_b);
        }

        for(std::size_t i = 0; i < collisions.size(); i++)
        {
            const CollisionSituation& situation = collisions[i];
            DeltaPair delta = deltas[i];
            if(is_kinematic)
            {
                situation.a.transform.position += delta.first;
                situation.b.transform.position += delta.second;
            }
        }
    }
}