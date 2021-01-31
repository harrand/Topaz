#ifndef TOPAZ_ALGO_NOISE_HPP
#define TOPAZ_ALGO_NOISE_HPP
#include <random>

namespace tz::algo
{
    /**
    * \addtogroup tz_algo Topaz Algorithms Library (tz::algo)
    * Contains common algorithms used in Topaz modules that aren't available in the C++ standard library.
    * @{
    */

    namespace prng
    {
        /**
		* \addtogroup tz_algo_prng Topaz PRNG Library (tz::algo::prng)
		* Contains some pre-defined random number generator engines, such as the Mersenne Twister.
		* @{
		*/
        /// 32-bit Mersenne Twister by Matsumoto and Nishimura, 1998
        using MersenneTwister = std::mt19937;
        /// "Minimum standard", recommended by Park, Miller, and Stockmeyer in 1993
        using LinearCongruential = std::minstd_rand;
        /// A shuffle-order engine based upon an older version of the LinearCongruential engine.
        using Knuth = std::knuth_b;
    }

    /**
     * Represents a map of random values based upon a seed. It can be indexed into as if it were an infinite image.
     * Note: The noise strategy affects the 'smoothness' of the resulting map, not the 'randomness' which is affected by the underlying random engine. Smoother maps are more applicable to terrain-generation, for example.
     * @tparam EngineType PRNG Engine Type to drive the noise. By default, this is a prng::Knuth. A prng::MersenneTwister may be preferred for some cases.
     */
    template<typename EngineType = prng::Knuth>
    class NoiseMap
    {  
    public:
        /**
         * Construct a NoiseMap based upon a given seed.
         * @param seed Seed of the underlying PRNG. NoiseMaps sharing the same seed will always generate the same values at any given coordinate.
         */
        NoiseMap(unsigned int seed);
        /**
         * Construct a NoiseMap by randomly-generating a seed and using that to drive the PRNG.
         */
        NoiseMap();

        /**
         * Retrieve a random float using rough randomness. Expect extremely spiky results.
         * Note: This is the fastest approach.
         * @param x X-coordinate of the map to retrieve.
         * @param z Z-coordinate of the map to retrieve.
         * @return Random value between 0.0f-1.0f.
         */
        float random(float x, float z);
        /**
         * Retrieve a random float using smooth randomness. Expect reasonably spiky results.
         * Note: This is a much slower approach than rough randomness, but produces smoother results.
         * @param x X-coordinate of the map to retrieve.
         * @param z Z-coordinate of the map to retrieve.
         * @return Random value between 0.0f-1.0f.
         */
        float smooth(float x, float z);
        /**
         * Retrieve a random float using smoothed randomness with cosine interpolation. Expect smooth results.
         * Note: This is easily the slowest approach, but produces the smoothest results by far.
         * @param x X-coordinate of the map to retrieve.
         * @param z Z-coordinate of the map to retrieve.
         * @return Random value between 0.0f-1.0f.
         */
        float cosine(float x, float z);
    private:
        unsigned int seed;
        EngineType engine;
    };
}

#include "algo/noise.inl"
#endif // TOPAZ_ALGO_NOISE_HPP