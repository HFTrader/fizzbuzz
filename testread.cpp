#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct State {
    const char TYPE[15] = {'N', 'N', 'F', 'N', 'B', 'F', 'N', 'N', 'F', 'B', 'N', 'F', 'N', 'N', 'Z'};
    uint64_t expected = 1;
    char curtype = 'N';

    uint32_t process(const char* ptr, uint32_t size) {
        const char* pend = ptr + size;
        const char* pstart = ptr;
        uint64_t value = 0;
        for (const char* p = ptr; p < pend; ++p) {
            if (*p == '\n') {
                switch (curtype) {
                    case 'N':
                        if (value != expected) {
                            fprintf(stderr, "Error expected:%ld got:%ld\n", expected, value);
                        }
                        break;
                    case 'F':
                        if ((p - pstart != 4) && (::memcmp("Fizz", pstart, 4) != 0)) {
                            fprintf(stderr, "Expected [Fizz] got [%.*s]\n", int(p - pstart), pstart);
                        }
                        break;
                    case 'B':
                        if ((p - pstart != 4) && (::memcmp("Buzz", pstart, 4) != 0)) {
                            fprintf(stderr, "Expected [Buzz] got [%.*s]\n", int(p - pstart), pstart);
                        }
                        break;
                    case 'Z':
                        if ((p - pstart != 8) && (::memcmp("FizzBuzz", pstart, 8) != 0)) {
                            fprintf(stderr, "Expected [FizzBuzz] got [%.*s]\n", int(p - pstart), pstart);
                        }
                        break;
                }
                pstart = p + 1;
                ++expected;
                curtype = TYPE[(expected - 1) % 15];
                value = 0;
            } else if (curtype == 'N') {
                value = value * 10 + (*p - '0');
            }
        }
        return pstart - ptr;
    }
};

struct Buffer {
    uint32_t minsize = 1024;
    uint32_t rdpos = 0;
    uint32_t wrpos = 0;
    uint32_t bufsize = 0;
    char* data = nullptr;
    Buffer(uint32_t size) : bufsize(size) {
        data = (char*)malloc(bufsize);
    }
    char* readptr() const {
        return data + rdpos;
    }
    char* writeptr() const {
        return data + wrpos;
    }
    uint32_t avail() const {
        return bufsize - wrpos;
    }
    uint32_t size() const {
        return wrpos - rdpos;
    }
    void commit(uint32_t nbytes) {
        wrpos += nbytes;
        // pack if necessary
        uint32_t left = bufsize - wrpos;
        if (left < minsize) {
            if (wrpos > rdpos) {
                ::memmove(data, data + rdpos, wrpos - rdpos);
            }
            wrpos = wrpos - rdpos;
            rdpos = 0;
        }
    }
    void advance(uint32_t nbytes) {
        rdpos += nbytes;
        if (rdpos == wrpos) {
            rdpos = wrpos = 0;
        }
    }
};

int main() {
    State state;
    uint32_t fd = fileno(stdin);
    Buffer buffer(1024 * 1024);
    while (true) {
        // read
        ssize_t nb = ::read(fd, buffer.writeptr(), buffer.avail());
        // fprintf(stderr, "Got %ld bytes", nb);
        if (nb > 0) {
            buffer.commit(nb);
            uint32_t nbytes = state.process(buffer.readptr(), buffer.size());
            buffer.advance(nbytes);
        } else if (nb < 0) {
            perror("read");
            break;
        }
    }
}