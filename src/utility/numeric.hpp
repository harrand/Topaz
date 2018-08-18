#ifndef NUMERIC_UTILITY_HPP
#define NUMERIC_UTILITY_HPP
#include <cmath>
#include <random>

namespace tz::utility::numeric
{
    float linear_interpolate(float a, float b, float blend_factor);
    float cosine_interpolate(float a, float b, float blend_factor);
    constexpr float default_smoothness = 8.0f;
    namespace consts
    {
        /// 3.14159...
        constexpr double pi = 3.14159265359;
        /// 2pi...
        constexpr double tau = 2.0 * pi;
        /// 2.17...
        constexpr double e = 2.71828182845;
        ///  1 / (2 - cbrt(2))
        constexpr double forest_ruth_coefficient = 1.35120719196;
        ///  1 - (2 * forest_ruth_coefficient)
        constexpr double forest_ruth_complement = -1.70241438392;
    }
}

/**
 * Object to generate random numbers with a given RNG-engine.
 * @tparam Engine - Random engine device
 * @tparam EngineResultType - Return type of the engine's productions
 */
template<typename Engine = std::default_random_engine, typename EngineResultType = std::default_random_engine::result_type>
class Random
{
public:
    typedef EngineResultType seed_type;
    typedef Engine engine_type;
    /**
     * Generate a Random from a seed.
     * @param seed - The seed for the RNG engine
     */
    Random<Engine, EngineResultType>(EngineResultType seed = std::random_device()());
    /**
     * Construct a Random from an existing, using their seed and engine.
     * @param copy - Random object to copy the seed and engine from.
     */
    Random<Engine, EngineResultType>(const Random<Engine, EngineResultType>& copy);

    /**
     * Get the value of the seed to this engine.
     * @return - The seed value
     */
    const EngineResultType& get_seed() const;
    /**
     * Read-only access to the underlying random engine.
     * @return - The random engine being used
     */
    const Engine& get_engine() const;
    /**
     * Generate a random signed integer between specified limits.
     * @param min - The minimum result of the integer
     * @param max - The maximum result of the integer
     * @return
     */
    int next_int(int min = 0, int max = std::numeric_limits<int>::max());
    /**
     * Generate a random float between specified limits.
     * @param min - The minimum result of the float
     * @param max - The maximum result of the float
     * @return
     */
    float next_float(float min = 0, float max = std::numeric_limits<float>::max());
    /**
     * Generate a random number between specified limits.
     * @tparam Number - The type of value to generate
     * @param min - The minimum result of the number
     * @param max - The maximum result of the number
     * @return
     */
    template <typename Number = int>
    inline Number operator()(Number min = Number(), Number max = std::numeric_limits<Number>::max());
private:
    /// Stores the seed used for this Random object.
    EngineResultType seed;
    /// Stores the underlying RNG engine for this Random object.
    Engine random_engine;
};

using LocalRandom = Random<>;
/**
* Template specialisation of Random, using the C++ mersenne-twister functionality.
* More expensive than a linear-congruential approach, but does provide higher-quality pseudorandomness.
*/
using MersenneTwister = Random<std::mt19937, std::mt19937::result_type>;

class SmoothNoise
{
public:
    SmoothNoise(int seed);
    float operator()(int x, int z);
protected:
    float base_noise(int x, int z);
    float smooth_noise(int x, int z);
private:
    int seed;
    LocalRandom random;
};

class CosineNoise : protected SmoothNoise
{
public:
    CosineNoise(int seed);
    float operator()(int x, int z, float smoothness = tz::utility::numeric::default_smoothness);
    float cosine_noise(int x, int z, float smoothness = tz::utility::numeric::default_smoothness);
protected:
    using SmoothNoise::operator();
};

#include "numeric.inl"
#endif //NUMERIC_UTILITY_HPP