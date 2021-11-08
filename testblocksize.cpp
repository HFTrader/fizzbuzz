#include "NumericUtils.h"
#include "MemUtils.h"
#include "Vanilla.h"
#include <iostream>
#include <string.h>

void test(uint64_t num) {
    char buffer[512];
    uint32_t realsize = vanilla(num, buffer);
    uint32_t calcsize = calcBlockSize(num);
    if (realsize != calcsize) {
        fprintf(stderr, "Error base:%ld real:%d calc:%d\n", num, realsize, calcsize);
        dump((uint8_t*)buffer, realsize);
    }
}

int main() {
    test(10);
    for (uint64_t num = 1; num < 1000000000ULL; ++num) {
        test(num);
    }
    for (uint32_t j = 1; j < 64; ++j) {
        for (int32_t k = -1; k < 2; ++k) {
            uint64_t value = (1ULL << j) + k;
            test(value);
        }
    }
}