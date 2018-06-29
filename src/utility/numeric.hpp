#ifndef NUMERIC_UTILITY_HPP
#define NUMERIC_UTILITY_HPP
#include <cmath>
#include <random>

namespace tz::utility::numeric
{
    namespace consts
    {
        /// 3.14159...
        constexpr double pi = 4 * std::atan(1);
        /// 2.17...
        constexpr double e = std::exp(1);
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
    const EngineResultType seed;
    /// Stores the underlying RNG engine for this Random object.
    Engine random_engine;
};

/**
* Template specialisation of Random, using the C++ mersenne-twister functionality.
* More expensive than a linear-congruential approach, but does provide higher-quality pseudorandomness.
*/
using MersenneTwister = Random<std::mt19937, std::mt19937::result_type>;

#include "numeric.inl"

#endif //NUMERIC_UTILITY_HPP
