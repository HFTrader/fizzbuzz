
#pragma once

#include "Chronometer.h"
#include <vector>
#include <limits>

/** 
 * Used to compute a simple average for steps through a sequence of events. 
 * This is simpler and less intrusive r using a full blown valgrind or vtune.
 * As such, it can be left in release binaries for monitoring.
 * We are not really worried about TSC rollover (which happens) since this is only informative.
 */
class LapTimer
{
    /** Will hold our stats for each step */
    struct Bin
    {
        double sum = 0;     //! Sum of all event points
        uint32_t count = 0; //! Counts number of points
        uint64_t start = 0; //! Holds the TSC reading at the last start point
        std::string name;   //! Name of this step
    };
    std::vector<Bin> bins; //! Holds all the steps
    uint64_t interval;     //! Print interval
    uint64_t nextprint;    //! Indicates when (TSC) we will print next
    std::string name;      //! Name of this sequence
    uint32_t last;         //! Last step input

public:
    //! Initializes this object with a name, step definitions and a print timeout
    LapTimer(const std::string &message, const std::vector<std::string> &names, uint64_t timeout)
    {
        name = message;
        interval = timeout;
        nextprint = interval > 0 ? ticks() + interval : std::numeric_limits<uint64_t>::max();
        bins.resize(names.size());
        for (uint32_t j = 0; j < names.size(); ++j)
        {
            bins[j].name = names[j];
        }
        last = 0;
    }

    //! Will print when timeout is over
    //! This method is public for if timeout is zero, it will not print so it can be called manually
    void print()
    {
        std::ostringstream oss;
        oss << "[" << name << "] ";
        for (uint32_t j = 0; j < bins.size(); ++j)
        {
            Bin &b(bins[j]);
            uint64_t avg = b.sum / b.count;
            oss << b.name << ":" << avg << " ";
            b.sum = 0;
            b.count = 0;
        }
        std::cerr << oss.str() << "\n";
    }

    //! Add a new time point
    //! step refers to the index of that location
    //! for this to work, it must be called in a perfect sequence: 0,1,2,0,1,2,0,1,2
    void lap(uint32_t step)
    {
        // Sanity check
        uint32_t prev = step > 0 ? step - 1 : bins.size() - 1;
        uint64_t now = ticks();
        bins[step].start = now;
        if (prev == last)
        {
            // Compute the difference between the previous and the current timestamp
            // Note that last==0 at start so the first point will not be computed (as expected)
            uint64_t delta = now - bins[prev].start;
            Bin &b(bins[step]);
            b.sum += delta;
            b.count++;
        }
        else
        {
            if (last != 0)
            {
                // If you call with the wrong sequence you'll know very fast
                std::cerr << "LapTimer[" << name << "]: expected " << last << " got " << prev << "\n";
            }
        }
        // For every start of sequence, check if we need to print
        if (step == 0)
        {
            if (now > nextprint)
            {
                // Compute the next time and print
                nextprint = now + interval;
                print();
            }
            // Reset the counter to eliminate the print time, which can be very big
            bins[step].start = ticks();
        }
        // Remember the last step - for sanity purposes
        last = step;
    }
};