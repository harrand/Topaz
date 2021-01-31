#ifndef TOPAZ_ALGO_NOISE_HPP
#define TOPAZ_ALGO_NOISE_HPP
#include <random>

namespace tz::algo
{
    namespace prng
    {
        using MersenneTwister = std::mt19937;
        using LinearCongruential = std::minstd_rand;
        using Knuth = std::knuth_b;
    }

    template<typename EngineType = prng::Knuth>
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
        EngineType engine;
    };
}

#include "algo/noise.inl"
#endif // TOPAZ_ALGO_NOISE_HPP