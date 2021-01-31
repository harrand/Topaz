#ifndef TOPAZ_ALGO_NOISE_HPP
#define TOPAZ_ALGO_NOISE_HPP
#include <random>

namespace tz::algo
{
    class NoiseMap
    {  
    public:
        NoiseMap(unsigned int seed);
        NoiseMap();

        float random(float x, float z);
        float smooth(float x, float z);
        float cosine(float x, float z);
    private:
        unsigned int seed;
        std::mt19937 engine;
    };
}

#endif // TOPAZ_ALGO_NOISE_HPP