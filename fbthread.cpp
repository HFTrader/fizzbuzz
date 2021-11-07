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

static uint32_t getBlockSize(uint64_t base) {
    uint32_t numdigits;
    uint64_t nextpow10;
    std::tie(numdigits, nextpow10) = vlog10(base);
    const uint32_t numlf = 15;
    const uint32_t numalpha = 6 * 4 + 1 * 8;
    uint32_t diff = nextpow10 - base;
    if (diff >= 15) {
        return numlf + numalpha + numdigits * 8;
    }
    const std::array<uint16_t, 15> nfirst{0, 1, 2, 2, 3, 3, 3, 4, 5, 5, 5, 6, 6, 7, 8};
    uint32_t nbefore = nfirst[diff];
    return numlf + numalpha + numdigits * nbefore + (numdigits + 1) * (8 - nbefore);
}

uint32_t vanilla(uint64_t base, char *p) {
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

struct BCD {
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
    BCD &operator+=(uint32_t value) {
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

using Flag = std::atomic<uint32_t>;

struct Block {
    uint64_t base;
    char *data;
    uint32_t size;
    uint32_t index;
};

struct Worker {
    uint32_t index;
    uint32_t start;
    uint32_t numblocks;
    char *data;
    uint32_t size;
    std::thread thread;
    Flag done;
    void run(Block *blocks);
};

void Worker::run(Block *blocks) {
    while (true) {
        while (done.load(std::memory_order_acquire) != 0) _mm_pause();
        for (uint32_t j = 0; j < numblocks; ++j) {
            Block &block(blocks[start + j]);
            vanilla(block.base, block.data);
        }
        done.store(1, std::memory_order_release);
    }
}

class Generator {
    uint32_t first;
    uint32_t index;
    uint32_t thbytes;
    uint32_t bytes;
    char *ptr;
    uint32_t blkid;
    uint64_t cyclebase;

public:
    Generator(char *buffer) {
        first = 0;
        index = 0;
        thbytes = 0;
        bytes = 0;
        ptr = buffer;
        blkid = 0;
        cyclebase = 1;
    }

    void prepare() {
        uint64_t base = cyclebase + blkid * 15;
        uint32_t blksize = getBlockSize(base);
        if (bytes + blksize > memsize) {
            break;
        }
        if ((blkid > first) && (thbytes + blksize > threadsize)) {
            threads[index].data = blocks[first].data;
            threads[index].size = ptr - blocks[first].data;
            threads[index].start = first;
            threads[index].numblocks = blkid - first;
            threads[index].done.store(0, std::memory_order_release);
            thbytes = 0;
            first = blkid;
            ++index;
        }
        Block &block(blocks[blkid]);
        block.base = base;
        block.data = ptr;
        block.size = blksize;
        block.index = index;
        ptr += blksize;
        bytes += blksize;
        thbytes += blksize;
    }

    void check() {
        // Loop through only the used threads (index not nthreads)
        for (uint32_t j = 0; j < index; ++j) {
            Worker &th(threads[j]);
            while (th.done.load(std::memory_order_acquire) == 0) _mm_pause();
            uint32_t nbytes = 0;
            while (nbytes < th.size) {
                iovec iov;
                iov.iov_base = th.data + nbytes;
                iov.iov_len = th.size - nbytes;
                ssize_t nb = ::vmsplice(fileno(stdout), &iov, 1, 0);
                if (nb > 0) {
                    nbytes += nb;
                } else {
                    std::cerr << "Error" << ::strerror(errno) << std::endl;
                }
            }
            // ssize_t nbytes = ::write(fileno(stdout), th.data, th.size);
            assert(nbytes == th.size);
        }
    }
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: fbthread <numthreads> <numblocks> \n");
        return 0;
    }

    uint32_t nthreads = std::atoi(argv[1]);
    uint32_t maxblocks = std::atoi(argv[2]);
    uint32_t memsize = getL2CacheSize() / 2;
    char *buffer = (char *)vmalloc(memsize);
    uint32_t threadsize = memsize / nthreads;

    std::vector<Block> blocks(maxblocks);
    std::cerr << "MemSize:" << memsize << " PerThread:" << threadsize << std::endl;
    LapTimer timer{"Blockgen", {"Cycle", "Generate", "Collect"}, 5 * 3200000000};

    Flag sync;
    sync.store(nthreads, std::memory_order_release);

    std::vector<Worker> threads(nthreads);
    for (uint32_t j = 0; j < nthreads; ++j) {
        Worker &th(threads[j]);
        th.done.store(1, std::memory_order_release);
        th.thread = std::thread(&Worker::run, &th, &blocks[0]);
    }
    Generator gen(buffer);
    while (true) {
        gen.prepare();
        gen.write();
    }
}