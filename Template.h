#pragma once
#include <cstdint>
#include "Vanilla.h"

/** The ASCII representation of any number with the indicated amount of digits */
template <unsigned NDIG>
struct Number {
    char data[NDIG];
    void set(uint64_t num) {
        for (uint32_t j = 0; j < NDIG; ++j) {
            data[NDIG - j - 1] = num % 10 + '0';
            num /= 10;
        }
    }
    void increment(uint32_t num) {
        uint32_t j = 0;
        while (num > 0) {
            num += data[NDIG - j - 1] - '0';
            data[NDIG - j - 1] = (num % 10) + '0';
            num /= 10;
            j++;
        }
    }
} __attribute__((packed));

/** The templated representation of one FizzBuzz block of 15 numbers */
template <unsigned NDIG>
struct Template {
    Number<NDIG> n1;
    char LF1;
    Number<NDIG> n2;
    char LF2;
    char fizz1[5];
    Number<NDIG> n4;
    char LF3;
    char buzz1[5];
    char fizz2[5];
    Number<NDIG> n7;
    char LF4;
    Number<NDIG> n8;
    char LF5;
    char fizz3[5];
    char buzz2[5];
    Number<NDIG> n11;
    char LF6;
    char fizz4[5];
    Number<NDIG> n13;
    char LF7;
    Number<NDIG> n14;
    char LF8;
    char fizzbuzz[9];

    /** Initializes all the constant terms of this block minus the actual numbers */
    Template() {
        LF1 = LF2 = LF3 = LF4 = LF5 = LF6 = LF7 = LF8 = '\n';
        std::memcpy(fizz1, "Fizz\n", 5);
        std::memcpy(fizz2, "Fizz\n", 5);
        std::memcpy(fizz3, "Fizz\n", 5);
        std::memcpy(fizz4, "Fizz\n", 5);
        std::memcpy(buzz1, "Buzz\n", 5);
        std::memcpy(buzz2, "Buzz\n", 5);
        std::memcpy(fizzbuzz, "FizzBuzz\n", 9);
        reset(ipow10(NDIG));
    }
    void reset(uint64_t base) {
        n1.set(base);
        n2.set(base + 1);
        n4.set(base + 3);
        n7.set(base + 6);
        n8.set(base + 7);
        n11.set(base + 10);
        n13.set(base + 12);
        n14.set(base + 13);
    }
    void increment(uint32_t value) {
        n1.increment(value);
        n2.increment(value);
        n4.increment(value);
        n7.increment(value);
        n8.increment(value);
        n11.increment(value);
        n13.increment(value);
        n14.increment(value);
    }

    // Fills the block starting with the number base
    uint32_t fill(uint64_t base, char *ptr) {
        reset(base);
        if (sizeof(*this) != calcBlockSize(base)) {
            fprintf(stderr, "Error base %ld expected %ld got %d\n", base, sizeof(*this), calcBlockSize(base));
        }
        std::memcpy(ptr, (void *)this, sizeof(*this));
        return sizeof(*this);
    }

    // Fills the block incrementing the BCD number only
    uint32_t incfill(uint32_t delta, char *ptr) {
        increment(delta);
        std::memcpy(ptr, (void *)this, sizeof(*this));
        return sizeof(*this);
    }
} __attribute__((packed));

/** Represents all possible templates with all numbers of digits.
 * Basically this was meant to encapsulate the huge switches  below */
struct TemplateBank {
    Template<1> t1;
    Template<2> t2;
    Template<3> t3;
    Template<4> t4;
    Template<5> t5;
    Template<6> t6;
    Template<7> t7;
    Template<8> t8;
    Template<9> t9;
    Template<10> t10;
    Template<11> t11;
    Template<12> t12;
    Template<13> t13;
    Template<14> t14;
    Template<15> t15;
    Template<16> t16;
    Template<17> t17;
    Template<18> t18;
    Template<19> t19;
    Template<20> t20;

    uint64_t base;       // Base used in last operation
    uint32_t ndigits;    // Number of digits last operation
    uint64_t nextpow10;  // Next power of 10 from last operation

    /** Fill the respective template and returns the length of the ascii representation */
    uint32_t fill(uint64_t number, char *ptr) {
        base = number;
        std::tie(ndigits, nextpow10) = vlog10(base);
        if (base + 15 - 1 < nextpow10) {
            switch (ndigits) {
                case 1: return t1.fill(base, ptr); break;
                case 2: return t2.fill(base, ptr); break;
                case 3: return t3.fill(base, ptr); break;
                case 4: return t4.fill(base, ptr); break;
                case 5: return t5.fill(base, ptr); break;
                case 6: return t6.fill(base, ptr); break;
                case 7: return t7.fill(base, ptr); break;
                case 8: return t8.fill(base, ptr); break;
                case 9: return t9.fill(base, ptr); break;
                case 10: return t10.fill(base, ptr); break;
                case 11: return t11.fill(base, ptr); break;
                case 12: return t12.fill(base, ptr); break;
                case 13: return t13.fill(base, ptr); break;
                case 14: return t14.fill(base, ptr); break;
                case 15: return t15.fill(base, ptr); break;
                case 16: return t16.fill(base, ptr); break;
                case 17: return t17.fill(base, ptr); break;
                case 18: return t18.fill(base, ptr); break;
                case 19: return t19.fill(base, ptr); break;
                case 20: return t20.fill(base, ptr); break;
            }
        }
        return vanilla(base, ptr);
    }

    /** Fills the respective template by adding a value.
     * A bit complex logic to keep everything in sync. */
    uint32_t incfill(uint32_t delta, char *ptr) {
        base += delta;
        uint32_t ndig = digits(base);
        if (ndig == digits(base + 15 - 1)) {
            if (ndig != ndigits) {
                ndigits = ndig;
                return fill(base, ptr);
            }
            switch (ndig) {
                case 1: return t1.incfill(delta, ptr); break;
                case 2: return t2.incfill(delta, ptr); break;
                case 3: return t3.incfill(delta, ptr); break;
                case 4: return t4.incfill(delta, ptr); break;
                case 5: return t5.incfill(delta, ptr); break;
                case 6: return t6.incfill(delta, ptr); break;
                case 7: return t7.incfill(delta, ptr); break;
                case 8: return t8.incfill(delta, ptr); break;
                case 9: return t9.incfill(delta, ptr); break;
                case 10: return t10.incfill(delta, ptr); break;
                case 11: return t11.incfill(delta, ptr); break;
                case 12: return t12.incfill(delta, ptr); break;
                case 13: return t13.incfill(delta, ptr); break;
                case 14: return t14.incfill(delta, ptr); break;
                case 15: return t15.incfill(delta, ptr); break;
                case 16: return t16.incfill(delta, ptr); break;
                case 17: return t17.incfill(delta, ptr); break;
                case 18: return t18.incfill(delta, ptr); break;
                case 19: return t19.incfill(delta, ptr); break;
                case 20: return t20.incfill(delta, ptr); break;
            }
        }
        return vanilla(base, ptr);
    }
};
