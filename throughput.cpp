#define _GNU_SOURCE 1
#include <cstdint>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/ioctl.h>

#include <sys/mman.h>
#include "NumericUtils.h"
#include "MemUtils.h"

#define TRY(cmd)             \
    do                       \
    {                        \
        int res = cmd;       \
        if (res < 0)         \
        {                    \
            int err = errno; \
            perror(#cmd);    \
            return 1;        \
        }                    \
    } while (0);

uint64_t now()
{
    return __builtin_ia32_rdtsc();
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: throughput <pipesize> <delay>\n");
        return 0;
    }
    uint32_t pipesize = ::atoi(argv[1]);
    uint32_t delaysecs = ::atoi(argv[2]);
    uint64_t delay = delaysecs * 3200000000ULL;
    uint32_t tosend = 1 * 1024 * 1024;
    char *buffer = (char *)vmalloc(tosend);
    for (uint32_t count = 0, nb = 0; nb < tosend;)
    {
        uint32_t left = tosend - nb;
        int32_t written = ::snprintf(&buffer[nb], left, "%d\n", ++count);
        if (written > 0)
        {
            if (written == left)
            {
                std::memset(&buffer[nb], '\n', written);
            }
            nb += written;
        }
    }
    fprintf(stderr, "Pipe size: %d\t\n\r", pipesize);
    uint32_t fd = fileno(stdout);
    TRY(::fcntl(fd, F_SETPIPE_SZ, pipesize));

    uint32_t maxpipe = getmaxpipe();

    uint64_t nextts = now() + delay;
    while (now() < nextts)
    {
        uint32_t nb = 0;
        while (nb < tosend)
        {
            iovec iov;
            iov.iov_base = &buffer[nb];
            iov.iov_len = tosend - nb;
            if (iov.iov_len > maxpipe)
            {
                iov.iov_len = maxpipe;
            }
            nb += ::vmsplice(fd, &iov, 1, 0);
        }
    }
}
