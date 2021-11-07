#include "NumericUtils.h"
#include <iostream>
#include <string.h>

void test(uint64_t num)
{
    uint32_t ndigits = numdigits(num);
    char buffer[32];
    int nd = ::sprintf(buffer, "%lu", num);
    int len = ::strlen(buffer);
    if ((ndigits != nd) || (ndigits != len))
    {
        std::cerr << num << "," << ndigits << "," << nd << "," << ::strlen(buffer) << std::endl;
    }
}

int main()
{
    for (uint64_t num = 0; num < 1000000000ULL; ++num)
    {
        test(num);
    }
    for (uint32_t j = 1; j < 64; ++j)
    {
        for (int32_t k = -1; k < 2; ++k)
        {
            uint64_t value = (1ULL << j) + k;
            test(value);
        }
    }
}