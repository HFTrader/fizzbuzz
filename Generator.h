#pragma once

#include <cstdint>
#include <array>
#include <thread>
#include "BCD.h"
#include "Buffer.h"
#include "PipeWriter.h"
#include "NumericUtils.h"

/** A fizzbuzz number generator with an embedded thread that feeds a PipeWriter */
struct Generator {
    uint64_t base;                  //! First number of the current 15-number block
    uint64_t blockjump;             //! How much to jump at the end of a block
    uint32_t counter;               //! Counts blocks (groups of 15 numbers)
    uint32_t numblocks;             //! Total number of blocks to generate before writing into pipe
    uint32_t numdigits;             //! Current number of digits on the numbers
    uint32_t numchars;              //! Number of characters in the current precomputed buffer
    std::array<BCD, 8> bcd;         //! Each fizzbuzz sequence of 15 lines has 8 actual numbers
    std::array<char, 1024> buffer;  //! Contains the ascii text for each precomputed buffer
    uint32_t offset;                //! Offset writing into the buffer
    BufferPtr stash;                //! Accumulates text as blocks are being generated. Passed to PipeWriter.
    PipeWriter &writer;             //! The object that actually writes to stdout on the main thread
    std::thread th;                 //! Thread encapsulated by this object
    LapTimer subtimer{"Submit", {"gen", "wait", "release"}, 5 * 3000000000};

    Generator(PipeWriter &w, uint64_t start, uint32_t nblocks, uint64_t incr)
        : base(start),
          blockjump(incr),
          counter(0),
          numblocks(nblocks),
          numdigits(0),
          numchars(0),
          offset(0),
          stash(w.request()),
          writer(w),
          th(&Generator::run, this) {
    }

    /** Runs indefinitely generating fizzbuzz blocks and pushing into the pipe writer */
    void run() {
        counter = 0;
        recalc();
        while (true) {
            writeblock();
        }
    }

    /** Recompute the block from scratch */
    void recalc() {
        if (digits(base) == digits(base + 15 - 1))
            precompute();
        else
            vanilla();
    }

    /** Computes one single block of 15 numbers of fizzbuzz */
    void advance(uint32_t delta) {
        base += delta;
        if (numdigits == digits(base + 15 - 1)) {
            // we have a precomputed sequence, just increment our BCDs
            for (BCD &bnum : bcd) bnum.increment(delta);
        } else if (digits(base) == digits(base + 15 - 1))
            precompute();
        else
            vanilla();
    }

    /** Saves the current block to our stash. When the number of blocks ends, writes into the pipe writer */
    uint32_t writeblock() {
        std::memcpy(&stash->data[offset], &buffer[0], numchars);
        offset += numchars;
        if (++counter < numblocks) {
            advance(15);
        } else {
            flush();
            advance(blockjump + 15);
        }
        return numchars;
    }

    /** Once all the blocks are generated, writes the resulting string into the pipe writer */
    void flush() {
        stash->used = offset;
        submit();
        offset = 0;
        counter = 0;
    }

    /** Submits a completed buffer to be printed out */
    void submit() {
        subtimer.lap(0);
        while (stash->flag.load(std::memory_order_acquire) != 0) _mm_pause();
        stash->flag.store(1, std::memory_order_release);
        subtimer.lap(1);
        while (stash->flag.load(std::memory_order_acquire) != 0) _mm_pause();
        subtimer.lap(2);
    }

    /** Slow routine used to generate a block when it crosses a boundary */
    void vanilla() {
        char *p = &buffer[0];
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
        numchars = p - &buffer[0];
        numdigits = 0;
    }

    /** Precomputes a block of 15 numbers for which all of them have the same number of digits.
     * If the block crosses a boundary (10,100,1000,etc) it will use the vanilla function instead. */
    void precompute() {
        numdigits = digits(base);
        buffer.fill('0');
        char *p = &buffer[0];
        // number 0
        bcd[0].init(p, base, numdigits);
        p += numdigits;
        *p++ = '\n';
        // number 1
        bcd[1].init(p, base + 1, numdigits);
        p += numdigits;
        *p++ = '\n';
        // number 2
        std::memcpy(p, "Fizz\n", 5);
        p += 5;
        // number 3
        bcd[2].init(p, base + 3, numdigits);
        p += numdigits;
        *p++ = '\n';
        // number 4
        std::memcpy(p, "Buzz\n", 5);
        p += 5;
        // number 5
        std::memcpy(p, "Fizz\n", 5);
        p += 5;
        // number 6
        bcd[3].init(p, base + 6, numdigits);
        p += numdigits;
        *p++ = '\n';
        // number 7
        bcd[4].init(p, base + 7, numdigits);
        p += numdigits;
        *p++ = '\n';
        // number 8
        std::memcpy(p, "Fizz\n", 5);
        p += 5;
        // number 9
        std::memcpy(p, "Buzz\n", 5);
        p += 5;
        // number 10
        bcd[5].init(p, base + 10, numdigits);
        p += numdigits;
        *p++ = '\n';
        // number 11
        std::memcpy(p, "Fizz\n", 5);
        p += 5;
        // number 12
        bcd[6].init(p, base + 12, numdigits);
        p += numdigits;
        *p++ = '\n';
        // number 13
        bcd[7].init(p, base + 13, numdigits);
        p += numdigits;
        *p++ = '\n';
        // number 14
        std::memcpy(p, "FizzBuzz\n", 9);
        p += 9;
        *p = '\0';
        numchars = p - &buffer[0];
        if (numchars > buffer.size()) {
            std::cerr << "Exceeded buffer size num:" << numchars << " max:" << buffer.size() << std::endl;
        }
    }
};
