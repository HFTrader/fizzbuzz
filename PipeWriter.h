#pragma once

#include <cstdint>
#include <cassert>
#include <vector>
#include "LapTimer.h"
#include "Buffer.h"
#include "MemUtils.h"
#include <immintrin.h>
#include <sys/ioctl.h>

/** Responsible for allocating buffers to the main thread and print them out when submitted */
class PipeWriter {
private:
    uint32_t numthreads;
    uint32_t index = 0;
    uint32_t blocksize = 0;
    std::vector<BufferPtr> avail;
    LapTimer timer{"PQueue", {"write", "wait"}, 5 * 3000000000};
    uint64_t lastval = 0;
    uint64_t lastdiff = 0;
    char *global_buffer;
    size_t global_buffer_size;

    struct Data {
        char *data;
        uint32_t size;
        std::atomic<uint32_t> &flag;
    };

    Data popqueue() {
        timer.lap(0);

        // Wait for the buffer to become ready (1)
        Buffer *b = avail[index].get();
        while (b->flag.load(std::memory_order_acquire) != 1) _mm_pause();

        Data data{b->data, b->used, b->flag};

        // Increment for next thread
        if (++index >= avail.size()) {
            index = 0;
        }

        // Releases the thread
        b->flag.store(2, std::memory_order_release);

        timer.lap(1);
        return data;
    }

    void sanity(const char *data) {
        uint64_t val = ::atoll(data);
        if (lastval > 0) {
            if (val < lastval) {
                std::cerr << "Out of order. " << val << " < " << lastval << std::endl;
            } else {
                uint64_t diff = val - lastval;
                if (diff % 15 != 0) {
                    std::cerr << "Difference between " << val << " and " << lastval << " is not a multiple of 15"
                              << std::endl;
                }
                if (lastdiff > 0) {
                    if (diff != lastdiff) {
                        std::cerr << "Difference " << diff << " is not equal to the previous " << lastdiff << std::endl;
                    }
                }
                lastdiff = diff;
            }
        } else {
            if (val != 1) {
                std::cerr << "First value has to be one, found " << val << std::endl;
            }
        }
        lastval = val;
    }

public:
    PipeWriter(uint32_t nthreads, uint32_t bufsize) {
        numthreads = nthreads;
        blocksize = roundtopages(bufsize);
        global_buffer_size = numthreads * blocksize;
        global_buffer = (char *)vmalloc(global_buffer_size);
        int res = madvise(global_buffer, global_buffer_size, MADV_HUGEPAGE);
        if (res < 0) {
            int res = errno;
            std::cerr << "madvise error: " << strerror(res) << std::endl;
        }
    }

    ~PipeWriter() {
        vmfree(global_buffer, global_buffer_size);
    }

    /** Thread runnable method to printout buffers in the queue and free main thread */
    void run() {
        int res = ::setvbuf(stdout, NULL, _IONBF, 0);
        if (res != 0) {
            int err = errno;
            std::cerr << "Failed removing buffer from stdout: " << strerror(err) << std::endl;
        }
        uint32_t fd = fileno(stdout);
        int32_t maxpipe = getmaxpipe();
        uint32_t desiredsize = maxpipe;
        bool haspipe = ::fcntl(fd, F_SETPIPE_SZ, desiredsize) >= 0;
        int32_t pipesize = ::fcntl(fd, F_GETPIPE_SZ);

        std::cerr << "This:" << this << " MaxPipe:" << maxpipe << " stdout:" << pipesize << std::endl;
        while (true) {
            Data data = popqueue();
            // Sanity check
            // sanity(data.data);
#if 0
            ssize_t nb = data.size;
#else
            ssize_t nb = 0;
            if (!haspipe) {
                while (nb < data.size) {
                    ssize_t res = ::write(fd, &data.data[nb], data.size - nb);
                    if (res >= 0) {
                        nb += res;
                    } else if (errno != EAGAIN) {
                        int err = errno;
                        std::cerr << "Error in write() nbytes:" << res << " error:" << strerror(err) << std::endl;
                    }
                }
            } else {
                while (nb < data.size) {
                    iovec iov;
                    iov.iov_base = &data.data[nb];
                    iov.iov_len = data.size - nb;
                    if (iov.iov_len > pipesize) {
                        // iov.iov_len = pipesize;
                    }
                    ssize_t res;
                    res = ::vmsplice(fd, &iov, 1, 0);

                    if (res > 0) {
                        nb += res;
                    } else if (res == 0) {
                        break;
                    } else if (errno != EAGAIN) {
                        int err = errno;
                        std::cerr << "Error in vmsplice nbytes:" << res << " error:" << strerror(err) << std::endl;
                    }
                }
            }
            if (nb != data.size) {
                std::cerr << "vmsplice expected " << data.size << " got " << nb << " bytes" << std::endl;
            }
#endif
            // Releases the thread
            data.flag.store(0, std::memory_order_release);
        }
    }

    /** Requests a free new or recycled buffer to be worked on */
    BufferPtr request() {
        uint32_t idx = avail.size();
        BufferPtr b(new Buffer);
        b->index = idx;
        b->size = blocksize;
        b->data = &global_buffer[idx * roundtopages(blocksize)];
        b->flag.store(0);
        avail.push_back(b);
        return b;
    }
};
