// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#pragma once

#include <cmath>
#include <cstdint>
#include <random>
#include <string>

/**
 * @brief A seeded, independent stream of random numbers
 *
 * Every random number in SedSAT3 comes from a RandomStream. The engine is
 * std::mt19937_64 seeded through std::seed_seq; both are fully specified by
 * the C++ standard, and the uniform and normal values are computed here from
 * the engine's raw output rather than with std::uniform_real_distribution and
 * std::normal_distribution, whose algorithms differ between standard
 * libraries. The same seed therefore gives the same numbers with GCC, Clang
 * and MSVC, on Linux, macOS and Windows.
 *
 * A stream is not thread-safe. Code that runs in parallel gives each chain or
 * thread its own stream, seeded from a master seed and the stream's index, so
 * results do not depend on how work is scheduled across threads.
 */
class RandomStream
{
public:
    /**
     * @brief Stream number @p stream derived from @p seed
     *
     * Different stream numbers with the same seed give statistically
     * independent sequences.
     */
    explicit RandomStream(std::uint64_t seed = 0, std::uint64_t stream = 0)
    {
        Reseed(seed, stream);
    }

    void Reseed(std::uint64_t seed, std::uint64_t stream = 0)
    {
        std::seed_seq sequence{
            static_cast<std::uint32_t>(seed), static_cast<std::uint32_t>(seed >> 32),
            static_cast<std::uint32_t>(stream), static_cast<std::uint32_t>(stream >> 32)};
        engine_.seed(sequence);
        has_spare_normal_ = false;
    }

    /// Uniform on [0, 1), with 53 random bits
    double Uniform()
    {
        return static_cast<double>(engine_() >> 11) * 0x1.0p-53;
    }

    /// Uniform on (0, 1], never zero, for taking logarithms
    double UniformPositive()
    {
        return static_cast<double>((engine_() >> 11) + 1) * 0x1.0p-53;
    }

    /// Standard normal, by the Box-Muller transform
    double StandardNormal()
    {
        if (has_spare_normal_)
        {
            has_spare_normal_ = false;
            return spare_normal_;
        }
        const double radius = std::sqrt(-2.0 * std::log(UniformPositive()));
        const double angle = 6.283185307179586476925 * Uniform();
        spare_normal_ = radius * std::sin(angle);
        has_spare_normal_ = true;
        return radius * std::cos(angle);
    }

    /// Normal with mean @p mean and standard deviation @p stdev
    double Normal(double mean, double stdev)
    {
        return mean + stdev * StandardNormal();
    }

    /// Uniform integer on [0, n), n > 0
    std::uint64_t Index(std::uint64_t n)
    {
        return static_cast<std::uint64_t>(Uniform() * static_cast<double>(n)) % n;
    }

private:
    std::mt19937_64 engine_;
    double spare_normal_ = 0.0;
    bool has_spare_normal_ = false;
};

/**
 * @brief A seed drawn from the operating system's entropy source
 *
 * Used when the user leaves the seed blank. It is 32 bits so that it is easy
 * to read back and type in to repeat a run.
 */
inline std::uint64_t DrawRandomSeed()
{
    std::random_device device;
    return static_cast<std::uint64_t>(device());
}

/**
 * @brief Parses a user-entered seed
 * @param text Decimal digits, or blank
 * @param seed Set to the parsed seed, or to a newly drawn one if @p text is blank
 * @return false if @p text is neither blank nor a non-negative integer that fits in 64 bits
 */
inline bool ParseRandomSeed(const std::string& text, std::uint64_t& seed)
{
    std::size_t begin = text.find_first_not_of(" \t");
    if (begin == std::string::npos)
    {
        seed = DrawRandomSeed();
        return true;
    }
    std::size_t end = text.find_last_not_of(" \t");
    const std::string digits = text.substr(begin, end - begin + 1);
    if (digits.find_first_not_of("0123456789") != std::string::npos || digits.size() > 20)
        return false;
    try
    {
        std::size_t used = 0;
        const unsigned long long value = std::stoull(digits, &used, 10);
        if (used != digits.size())
            return false;
        seed = static_cast<std::uint64_t>(value);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

/**
 * @brief The stream for code outside the MCMC that needs random numbers
 *
 * One per thread, so parallel code never shares a generator. Every thread's
 * stream starts from the same fixed seed (DefaultRandomSeed), so a run is
 * repeatable; code whose result must not depend on what ran before it (the
 * Levenberg-Marquardt starting point, Error Analysis) uses its own
 * RandomStream instead. Because every thread's stream starts from the same
 * seed, parallel code must not draw from it: give each chain or task its own
 * RandomStream(seed, index), as the MCMC does.
 */
constexpr std::uint64_t DefaultRandomSeed = 20261016;

inline RandomStream& ThreadRandomStream()
{
    thread_local RandomStream stream(DefaultRandomSeed);
    return stream;
}
