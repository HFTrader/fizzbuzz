#pragma once

#include <cstdint>
#include <array>
#include <tuple>

/** VERY slow way to compute the next power of 10. Use vlog10 instead. */
static uint64_t nextpow10(uint64_t num) {
    uint64_t base = 1;
    while (base <= num) {
        base *= 10;
    }
    return base;
}

/** VERY slow integer log10 for unit testing only. Use vlog10 instead. */
static uint32_t ilog10(uint64_t num) {
    uint64_t base = 1;
    uint32_t numdigits = 1;
    while (base * 10 <= num) {
        base *= 10;
        numdigits++;
    }
    return numdigits;
}

/** Slow power of 10 for initialization etc */
static uint64_t ipow10(uint32_t n) {
    uint64_t value = 1;
    for (uint32_t j = 0; j < n; ++j) {
        value *= 10;
    }
    return value;
}

/** Represents one entry in the power-of-2 to power-of-10 translation table */
struct Pow2Item {
    uint32_t digits;     // Number of digits of the first power of 2 in this sequence
    uint64_t nextpow10;  // Next power of 10 value that is greater than the base power2
};

// This table was created in python with
// import math
// vec =[ int(math.log10(2**j)) for j in range(64) ]
// print( [ (v+1,10**(v+1),2**j,2**(j+1)-1) for j,v in enumerate(vec) ] )
static std::array<Pow2Item, 64> levels{{{1, 10ULL},
                                        {1, 10ULL},
                                        {1, 10ULL},
                                        {1, 10ULL},
                                        {2, 100ULL},
                                        {2, 100ULL},
                                        {2, 100ULL},
                                        {3, 1000ULL},
                                        {3, 1000ULL},
                                        {3, 1000ULL},
                                        {4, 10000ULL},
                                        {4, 10000ULL},
                                        {4, 10000ULL},
                                        {4, 10000ULL},
                                        {5, 100000ULL},
                                        {5, 100000ULL},
                                        {5, 100000ULL},
                                        {6, 1000000ULL},
                                        {6, 1000000ULL},
                                        {6, 1000000ULL},
                                        {7, 10000000ULL},
                                        {7, 10000000ULL},
                                        {7, 10000000ULL},
                                        {7, 10000000ULL},
                                        {8, 100000000ULL},
                                        {8, 100000000ULL},
                                        {8, 100000000ULL},
                                        {9, 1000000000ULL},
                                        {9, 1000000000ULL},
                                        {9, 1000000000ULL},
                                        {10, 10000000000ULL},
                                        {10, 10000000000ULL},
                                        {10, 10000000000ULL},
                                        {10, 10000000000ULL},
                                        {11, 100000000000ULL},
                                        {11, 100000000000ULL},
                                        {11, 100000000000ULL},
                                        {12, 1000000000000ULL},
                                        {12, 1000000000000ULL},
                                        {12, 1000000000000ULL},
                                        {13, 10000000000000ULL},
                                        {13, 10000000000000ULL},
                                        {13, 10000000000000ULL},
                                        {13, 10000000000000ULL},
                                        {14, 100000000000000ULL},
                                        {14, 100000000000000ULL},
                                        {14, 100000000000000ULL},
                                        {15, 1000000000000000ULL},
                                        {15, 1000000000000000ULL},
                                        {15, 1000000000000000ULL},
                                        {16, 10000000000000000ULL},
                                        {16, 10000000000000000ULL},
                                        {16, 10000000000000000ULL},
                                        {16, 10000000000000000ULL},
                                        {17, 100000000000000000ULL},
                                        {17, 100000000000000000ULL},
                                        {17, 100000000000000000ULL},
                                        {18, 1000000000000000000ULL},
                                        {18, 1000000000000000000ULL},
                                        {18, 1000000000000000000ULL},
                                        {19, 10000000000000000000ULL},
                                        {19, 10000000000000000000ULL},
                                        {19, 10000000000000000000ULL},
                                        {19, 10000000000000000000ULL}}};

/** O(1) returns the number of digits and the next power of 10 given a number */
static std::tuple<uint32_t, uint64_t> vlog10(uint64_t num) {
    // Compute the exception because clz does not like zero (undefined)
    if (num == 0) return {1, 1};

    // clz returns the number of zeros high in a binary number so the position
    // of the highest bit is 64 minus that value. It's a 1 cycle integer log2.
    uint32_t numbits = 64 - __builtin_clzl(num);

    // Given the log2, we look it up in the pow10 table to find what power of
    // 10 it relates to.
    const Pow2Item &p2(levels[numbits - 1]);

    // If this number is less than the power of 10 in that table, the number
    // of digits is exactly the indicated in there
    if (num < p2.nextpow10) {
        return {p2.digits, p2.nextpow10};
    }

    // Otherwise is one digit more
    return {p2.digits + 1, p2.nextpow10 * 10};
}

/** Returns the number of digits in a given number without asciifying it */
static uint32_t digits(uint64_t num) {
    uint32_t numdigits;
    uint64_t nextpow10;
    std::tie(numdigits, nextpow10) = vlog10(num);
    return numdigits;
}

/** Computes the size of a 15-number stringified fizzbuzz block */
static uint32_t calcBlockSize(uint64_t base) {
    // Computes the number of digits of the first number in the block
    uint32_t numdigits;
    uint64_t nextpow10;
    std::tie(numdigits, nextpow10) = vlog10(base);

    // if the difference between the first number and the next power of 10 is greater or
    // equal than 15, all numbers in the block have the same number of digits (easy)
    const uint32_t numlf = 15;
    const uint32_t numalpha = 6 * 4 + 1 * 8;
    uint64_t diff = nextpow10 - base;
    if (diff >= 15) {
        return numlf + numalpha + numdigits * 8;
    }

    // We have a mix, need to compute how many before and after the next power of 10
    const std::array<uint16_t, 15> nfirst{0, 1, 2, 2, 3, 3, 3, 4, 5, 5, 5, 6, 6, 7, 8};
    uint32_t nbefore = nfirst[diff];
    return numlf + numalpha + numdigits * nbefore + (numdigits + 1) * (8 - nbefore);
}
