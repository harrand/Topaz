#include "algo/math.hpp"

namespace tz::algo
{
    template<typename EngineType>
    NoiseMap<EngineType>::NoiseMap(unsigned int seed): seed(seed), engine(seed){}

    template<typename EngineType>
    NoiseMap<EngineType>::NoiseMap(): seed(std::random_device{}()), engine(seed){}

    template<typename EngineType>
    float NoiseMap<EngineType>::random(float x, float z)
    {
        unsigned int temp_seed = x * 56346.03297 + z * 189793.3498 + this->seed;
        this->engine.seed(temp_seed);
        float random_result = std::uniform_real_distribution<float>(0.0f, 1.0f)(this->engine);
        return random_result;
    }

    template<typename EngineType>
    float NoiseMap<EngineType>::smooth(float x, float z)
    {
        float corners = (this->random(x - 1, z - 1) + this->random(x + 1, z - 1) + this->random(x - 1, z + 1) + this->random(x + 1, z + 1))/16.0f;
        float sides = (this->random(x - 1, z) + this->random(x + 1, z) + this->random(x, z + 1) + this->random(x, z - 1))/8.0f;
        float centre = this->random(x, z) / 4.0f;
        return corners + sides + centre;
    }

    template<typename EngineType>
    float NoiseMap<EngineType>::cosine(float x, float z)
    {
        auto ix = static_cast<int>(x);
        auto iz = static_cast<int>(z);
        float fx = x - ix;
        float fz = z - iz;

        float v1 = smooth(ix, iz);
        float v2 = smooth(ix + 1, iz);
        float v3 = smooth(ix, iz + 1);
        float v4 = smooth(ix + 1, iz + 1);
        float i1 = tz::algo::cosine_interpolate(v1, v2, fx);
        float i2 = tz::algo::cosine_interpolate(v3, v4, fx);
        return tz::algo::cosine_interpolate(i1, i2, fz);
    }
}