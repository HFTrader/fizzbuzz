#pragma once

#include <cstdint>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <fstream>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

/** Computes the next round size that fits on full pages */
static size_t roundtopages(size_t size) {
    size_t page_size = ::getpagesize();
    unsigned int pages = ((size - 1) / page_size) + 1;
    return pages * page_size;
}

/** Frees memory allocated iwth vmalloc */
static void vmfree(void *ptr, size_t size) {
    munmap(ptr, size);
}

/** Allocates size bytes of aligned, protected memory */
static void *vmalloc(size_t size) {
    size_t page_size = ::getpagesize();
    unsigned int pages = ((size - 1) / page_size) + 2;
    uint8_t *ret = (uint8_t *)mmap(NULL, page_size * pages, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    mprotect(ret + (pages - 1) * page_size, page_size, PROT_NONE);
    return ret;
}

/** Returns the maximum pipe buffer size */
static uint32_t getmaxpipe() {
    std::ifstream inp("/proc/sys/fs/pipe-max-size");
    uint32_t sz;
    inp >> sz;
    return sz;
}

static uint32_t getL2CacheSize() {
    std::ifstream inp("/sys/devices/system/cpu/cpu0/cache/index2/size");
    uint32_t sz;
    inp >> sz;
    char prefix;
    inp >> prefix;
    switch (prefix) {
        case 'K': sz *= 1024; break;
        case 'M': sz *= 1024 * 1024; break;
        case 'G': sz *= 1024 * 1024 * 1024; break;
    }
    return sz;
}

static void waitms(uint32_t msecs) {
    std::this_thread::sleep_for(std::chrono::milliseconds(msecs));
}

static void waitus(uint32_t msecs) {
    std::this_thread::sleep_for(std::chrono::microseconds(msecs));
}