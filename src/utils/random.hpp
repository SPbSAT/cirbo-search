#ifndef CIRBO_SEARCH_UTILS_RANDOM_HPP
#define CIRBO_SEARCH_UTILS_RANDOM_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>

namespace cirbo::utils
{

static constexpr uint64_t DefaultGlobalSeed = 8132751891241;

// TODO: Add Global Seed argument to main
class GlobalSeed
{
private:
    uint64_t SeedValue_ = DefaultGlobalSeed;

public:
    static GlobalSeed& getInstance()
    {
        static GlobalSeed instance;
        return instance;
    }

    static uint64_t get() { return GlobalSeed::getInstance().SeedValue_; }

    static void set(uint64_t value)
    {
        // Could be changed only once during a program.
        assert(get() == DefaultGlobalSeed);
        GlobalSeed::getInstance().SeedValue_ = value;
    };
};

/**
 * @return Next random value, determined by GlobalSeed.
 */
static uint64_t getNextRandomSeed()
{
    static std::mt19937 mtGen(GlobalSeed::get());
    static std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    return dist(mtGen);
}

/**
 * @return New Mersenne Twister Engine, seeded by predictable number.
 */
static std::mt19937 getNewMersenneTwisterEngine() { return std::mt19937(getNextRandomSeed()); }

/**
 * @brief Generates a random index in range [0, size).
 * @param size Upper bound of the range.
 * @param rng Random generator.
 * @return Random index.
 */
inline size_t randomIndex(size_t size, std::mt19937& rng)
{
    if (size == 0)
    {
        return 0;
    }

    std::uniform_int_distribution<size_t> dist(0, size - 1);
    return dist(rng);
}

/**
 * @brief Generates a random floating point value in range [0.0, 1.0].
 * @param rng Random generator.
 * @return Random double value.
 */
inline double randomDouble(std::mt19937& rng)
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

}  // namespace cirbo::utils

#endif  // CIRBO_SEARCH_UTILS_RANDOM_HPP
