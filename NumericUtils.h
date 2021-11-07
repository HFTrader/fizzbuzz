#pragma once

#include <cstdint>
#include <array>
#include <tuple>

static uint64_t nextpow10(uint64_t num)
{
    uint64_t base = 1;
    while (base <= num)
    {
        base *= 10;
    }
    return base;
}

static uint32_t log10(uint64_t num)
{
    uint64_t base = 1;
    uint32_t numdigits = 1;
    while (base * 10 <= num)
    {
        base *= 10;
        numdigits++;
    }
    return numdigits;
}

static std::tuple<uint32_t, uint64_t> vlog10(uint64_t num)
{
    struct Pow2Item
    {
        uint32_t digits;
        uint64_t nextpow10;
    };
    static std::array<Pow2Item, 64> levels{{{1, 10ULL}, {1, 10ULL}, {1, 10ULL}, {1, 10ULL}, {2, 100ULL}, {2, 100ULL}, {2, 100ULL}, {3, 1000ULL}, {3, 1000ULL}, {3, 1000ULL}, {4, 10000ULL}, {4, 10000ULL}, {4, 10000ULL}, {4, 10000ULL}, {5, 100000ULL}, {5, 100000ULL}, {5, 100000ULL}, {6, 1000000ULL}, {6, 1000000ULL}, {6, 1000000ULL}, {7, 10000000ULL}, {7, 10000000ULL}, {7, 10000000ULL}, {7, 10000000ULL}, {8, 100000000ULL}, {8, 100000000ULL}, {8, 100000000ULL}, {9, 1000000000ULL}, {9, 1000000000ULL}, {9, 1000000000ULL}, {10, 10000000000ULL}, {10, 10000000000ULL}, {10, 10000000000ULL}, {10, 10000000000ULL}, {11, 100000000000ULL}, {11, 100000000000ULL}, {11, 100000000000ULL}, {12, 1000000000000ULL}, {12, 1000000000000ULL}, {12, 1000000000000ULL}, {13, 10000000000000ULL}, {13, 10000000000000ULL}, {13, 10000000000000ULL}, {13, 10000000000000ULL}, {14, 100000000000000ULL}, {14, 100000000000000ULL}, {14, 100000000000000ULL}, {15, 1000000000000000ULL}, {15, 1000000000000000ULL}, {15, 1000000000000000ULL}, {16, 10000000000000000ULL}, {16, 10000000000000000ULL}, {16, 10000000000000000ULL}, {16, 10000000000000000ULL}, {17, 100000000000000000ULL}, {17, 100000000000000000ULL}, {17, 100000000000000000ULL}, {18, 1000000000000000000ULL}, {18, 1000000000000000000ULL}, {18, 1000000000000000000ULL}, {19, 10000000000000000000ULL}, {19, 10000000000000000000ULL}, {19, 10000000000000000000ULL}, {19, 10000000000000000000ULL}}};
    if (num == 0)
        return {1, 1};
    uint32_t numbits = 64 - __builtin_clzl(num);
    const Pow2Item &p2(levels[numbits - 1]);
    if (num < p2.nextpow10)
    {
        return {p2.digits, p2.nextpow10};
    }
    return {p2.digits + 1, p2.nextpow10};
}

static uint32_t digits(uint64_t num)
{
    uint32_t numdigits;
    uint64_t nextpow10;
    std::tie(numdigits, nextpow10) = vlog10(num);
    return numdigits;
}
