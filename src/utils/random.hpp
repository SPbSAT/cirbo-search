#ifndef CIRBO_SEARCH_UTILS_RANDOM_HPP
#define CIRBO_SEARCH_UTILS_RANDOM_HPP

#include <cassert>
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

}  // namespace cirbo::utils

#endif  // CIRBO_SEARCH_UTILS_RANDOM_HPP
