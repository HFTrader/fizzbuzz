
#pragma once
#include <cstdint>

static uint32_t vanilla(uint64_t base, char *p) {
    char *start = p;
    p += sprintf(p, "%ld\n", base);
    p += sprintf(p, "%ld\n", base + 1);
    p += sprintf(p, "Fizz\n");
    p += sprintf(p, "%ld\n", base + 3);
    p += sprintf(p, "Buzz\n");
    p += sprintf(p, "Fizz\n");
    p += sprintf(p, "%ld\n", base + 6);
    p += sprintf(p, "%ld\n", base + 7);
    p += sprintf(p, "Fizz\n");
    p += sprintf(p, "Buzz\n");
    p += sprintf(p, "%ld\n", base + 10);
    p += sprintf(p, "Fizz\n");
    p += sprintf(p, "%ld\n", base + 12);
    p += sprintf(p, "%ld\n", base + 13);
    p += sprintf(p, "FizzBuzz\n");
    uint32_t numchars = p - start;
    return numchars;
}
