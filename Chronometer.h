#pragma once

#include <cstdint>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>

/** Returns the CPU timestamp counter - for performance measurement only */
static uint64_t ticks()
{
    return __builtin_ia32_rdtsc();
}

static uint64_t now()
{
    timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return uint64_t(tp.tv_nsec) / 1000000 + uint64_t(tp.tv_sec) * 1000;
}

/** Computes throughput given number of bytes updates */
class Chronometer
{
private:
    uint64_t msecs_start = now();
    uint64_t msecs_lap = msecs_start;
    uint64_t tstart = ticks();
    uint64_t tlap = tstart;
    uint64_t timeout = 1000000000;
    uint64_t bytes = 0;
    uint64_t bytes_lap = 0;

public:
    /** Constructor is passed the timeout in ticks to print progress */
    Chronometer(uint64_t tmout) : timeout(tmout) { reset(); }

    /** Initializes internal state */
    void reset()
    {
        msecs_start = now();
        msecs_lap = msecs_start;
        tstart = ticks();
        tlap = tstart;
        bytes = 0;
        bytes_lap = 0;
    }

    /** inputs updates on progress */
    void lap(uint64_t nb)
    {
        bytes += nb;
        bytes_lap += nb;
        uint64_t tnow = ticks();
        if (tnow > tlap + timeout)
        {
            uint64_t msecs_now = now();
            uint64_t delta_start = msecs_now - msecs_start;
            uint64_t delta_lap = msecs_now - msecs_lap;
            fprintf(stderr, "Lap: %ld bytes throughput: %ld %ld\n", bytes, bytes_lap / delta_lap, bytes / delta_start);
            bytes_lap = 0;
            tlap = tnow;
            msecs_lap = msecs_now;
        }
    }
};
