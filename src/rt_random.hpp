//! Simple (and fast!) good-enough random generator implementations

#ifndef RT_RANDOM_HPP_
#define RT_RANDOM_HPP_

#include "rt_common.hpp"

/// Custom random generator
namespace rt::random {

    /// Splitmix64 random generator
    class splitmix64 {
    public:
        splitmix64(std::uint64_t seed): state(seed) {
        }

        std::uint64_t next() noexcept {
            std::uint64_t r = (state += 0x9E3779B97F4A7C15);
            r = (r ^ (r >> 30)) * 0xBF58476D1CE4E5B9;
            r = (r ^ (r >> 27)) * 0x94D049BB133111EB;
            return r ^ (r >> 31);
        }

    private:
        std::uint64_t state;
    };

    /// Xoshiro256++ random generator
    class xoshiro256pp {
    public:
        xoshiro256pp(std::uint64_t seed) noexcept {
            splitmix64 init(seed);

            s0 = init.next();
            s1 = init.next();
            s2 = init.next();
            s3 = init.next();
        }

        std::uint64_t next() noexcept {
            std::uint64_t result = s0 + std::rotl(s0 + s3, 23);
            std::uint64_t t = s1 << 17;

            s2 ^= s0;
            s3 ^= s1;
            s1 ^= s2;
            s0 ^= s3;

            s2 ^= t;
            s3 = std::rotl(s3, 45);

            return result;
        }

    private:
        std::uint64_t s0, s1, s2, s3;
    };
}

#endif // RT_RANDOM_HPP_

// rt_random.hpp
