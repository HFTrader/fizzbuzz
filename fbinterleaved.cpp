#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>
#include <atomic>
#include <iostream>
#include <cassert>
#include <thread>
#include <immintrin.h>
#include "MemUtils.h"
#include "NumericUtils.h"
#include "LapTimer.h"
#include "Vanilla.h"
#include "Template.h"

using Flag = std::atomic<uint32_t>;

const uint32_t MAXBLOCKSIZE = 300;  // max 15-number block size with 20 digits 64 bit integers

using Flag = std::atomic<uint32_t>;
struct Work {
    Flag flag;
    uint32_t idx;
    uint32_t nthreads;
    char **buffer_ptr;
    uint32_t bufsize;
    std::thread thread;
};

void generator(Work &work) {
    TemplateBank bank;
    uint64_t base = 1;
    uint32_t accumulated = 0;
    char *buffer = *work.buffer_ptr;
    bank.fill(1, buffer);
    while (true) {
        // Wait for release
        // fprintf(stderr, "\tThread %d waiting...\n", work.idx);
        while (work.flag.load() == 0) __builtin_ia32_pause();
        // fprintf(stderr, "\tThread %d released...\n", work.idx);
        uint32_t nbytes = 0;
        char *ptr = buffer;
        uint32_t numdigits = 1;
        while (ptr - buffer < work.bufsize - MAXBLOCKSIZE) {
            if ((base - 1) / 15 % work.nthreads == work.idx) {
                // generate block
                ptr += bank.incfill(accumulated, ptr);
                accumulated = 0;

            } else {
                // skip block
                ptr += calcBlockSize(base);
            }
            accumulated += 15;
            base += 15;
        }
        // Notify
        work.flag.store(0);
        // fprintf(stderr, "\tThread %d done\n", work.idx);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stdout, "Usage: fbthread <numthreads> \n");
        return 0;
    }
    // Allocate buffer
    uint32_t bufsize = getL2CacheSize() / 2;
    char *buffer = (char *)vmalloc(bufsize);
    int res = madvise(buffer, bufsize, MADV_HUGEPAGE);
    if (res < 0) {
        int res = errno;
        std::cerr << "madvise error: " << strerror(res) << std::endl;
    }

    // Adjust pipe size accordingly
    int32_t maxpipe = getmaxpipe();
    uint32_t desiredsize = bufsize < maxpipe ? bufsize : maxpipe;
    uint32_t fd = fileno(stdout);
    bool haspipe = ::fcntl(fd, F_SETPIPE_SZ, desiredsize) >= 0;
    int32_t pipesize = ::fcntl(fd, F_GETPIPE_SZ);

    // Create threads
    uint32_t numthreads = ::atoi(argv[1]);
    std::vector<Work> workers(numthreads);
    for (uint32_t j = 0; j < numthreads; ++j) {
        Work &work(workers[j]);
        work.bufsize = bufsize;
        work.flag.store(0);
        work.nthreads = numthreads;
        work.idx = j;
        work.thread = std::thread(&generator, std::ref(work));
        work.buffer_ptr = &buffer;
    }

    // Start loop
    uint64_t base = 1;
    while (true) {
        // Release threads
        // fprintf(stderr, "Main thread releasing threads...\n");
        for (uint32_t j = 0; j < numthreads; ++j) {
            workers[j].flag.store(1);
        }

        // Loop generating buffer
        uint32_t nbytestosend = 0;
        while (nbytestosend < bufsize - MAXBLOCKSIZE) {
            nbytestosend += calcBlockSize(base);
            base += 15;
        }

        // Wait for all threads to finish
        // fprintf(stderr, "Main thread waiting...\n");
        for (uint32_t j = 0; j < numthreads; ++j) {
            while (workers[j].flag.load() > 0) __builtin_ia32_pause();
        }
        // fprintf(stderr, "Main thread sending...\n");

        // Send buffer
        if (haspipe > 0) {
            iovec iov;
            uint32_t sentbytes = 0;
            while (sentbytes < nbytestosend) {
                iov.iov_base = &buffer[sentbytes];
                iov.iov_len = nbytestosend - sentbytes;
                ssize_t res = ::vmsplice(fd, &iov, 1, 0);
                if (res > 0) {
                    sentbytes += res;
                } else {
                    fprintf(stderr, "vmsplice: %s\n", strerror(errno));
                    return 1;
                }
            }
        } else {
            ssize_t res = ::write(fd, buffer, nbytestosend);
        }
    }
    return 0;
}
