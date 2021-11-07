#pragma once

/** 
 * Implements a vanilla BCD number and relevant operations.
 * There is an enormous amount of potential for optimization here.
 */
class BCD
{
private:
    char *digits;       ///< points to the END of the bcd number
    uint32_t numdigits; ///< number of digits actually used

public:
    //! Initializes the BCD cache with a given number
    //! ptr typically will point to a location inside the out buffer
    //! base is the actual number that this BCD represents
    //! ndigits is the number of digits
    //! This is slow and there is no point optimizing it as it's executed only once
    //! every time the number of digits changes
    void init(char *ptr, uint64_t base, uint32_t ndigits)
    {
        digits = ptr + (ndigits - 1);
        numdigits = ndigits;
        char *p = digits;
        while (p >= ptr)
        {
            *p = '0' + (base % 10);
            p--;
            base /= 10;
        }
    }

    //! Increments the BCD number by a given amount
    void increment(uint32_t carry)
    {
        char *p = digits;
        while (carry > 0)
        {
            uint32_t val = uint32_t(*p - '0') + carry;
            carry = val / 10;
            *p = (char)((val % 10) + '0');
            p--;
        }
    }

} __attribute__((packed));
