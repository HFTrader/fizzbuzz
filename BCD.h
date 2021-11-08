#pragma once

/**
 * Implements a vanilla BCD number and relevant operations.
 * There is an enormous amount of potential for optimization here.
 */
class BCD {
private:
    char *digits;        ///< points to the END of the bcd number
    uint32_t numdigits;  ///< number of digits actually used

public:
    //! Initializes the BCD cache with a given number
    //! ptr typically will point to a location inside the out buffer
    //! base is the actual number that this BCD represents
    //! ndigits is the number of digits
    //! This is slow and there is no point optimizing it as it's executed only once
    //! every time the number of digits changes
    void init(char *ptr, uint64_t base, uint32_t ndigits) {
        digits = ptr + (ndigits - 1);
        numdigits = ndigits;
        char *p = digits;
        while (p >= ptr) {
            *p = '0' + (base % 10);
            p--;
            base /= 10;
        }
    }

    //! Increments the BCD number by a given amount
    void increment(uint32_t carry) {
        char *p = digits;
        while (carry > 0) {
            uint32_t val = uint32_t(*p - '0') + carry;
            carry = val / 10;
            *p = (char)((val % 10) + '0');
            p--;
        }
    }

} __attribute__((packed));

static uint32_t BCDadd(uint32_t a, uint32_t b) {
    uint32_t t1, t2;  // unsigned 32-bit intermediate values

    t1 = a + 0x06666666;
    t2 = t1 ^ b;                 // sum without carry propagation
    t1 = t1 + b;                 // provisional sum
    t2 = t1 ^ t2;                // all the binary carry bits
    t2 = ~t2 & 0x11111110;       // just the BCD carry bits
    t2 = (t2 >> 2) | (t2 >> 3);  // correction
    return t1 - t2;              // corrected BCD sum
}

static uint64_t BCDpack(uint32_t value) {
    uint64_t bcd = 0;
    uint64_t p10 = 1;
    while (value > 0) {
        uint32_t rem = value % 10;
        bcd += rem * p10;
        value /= 10;
    }
    return bcd;
}

struct BCDPacked {
    using Packed = uint32_t;
    Packed pack[3];
    void set(uint64_t num) {
        const uint64_t quot = 10000000;
        pack[0] = BCDpack(num % quot);
        pack[1] = BCDpack((num / quot) % quot);
        pack[2] = BCDpack(num / (quot * quot));
    }
    void clear() {
        pack[0] = pack[1] = pack[2] = 0;
    }
    BCDPacked &operator+=(uint32_t value) {
        uint32_t bcdsum = BCDpack(value);
        bcdsum = BCDadd(bcdsum, pack[0]);
        pack[0] = bcdsum & 0x0FFFFFFF;
        bcdsum = BCDadd(bcdsum >> 28, pack[1]);
        pack[1] = bcdsum & 0x0FFFFFFF;
        bcdsum = BCDadd(bcdsum >> 28, pack[2]);
        pack[2] = bcdsum & 0x0FFFFFFF;
        return *this;
    }
};