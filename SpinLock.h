#pragma once
#include <atomic>

/** Simple non-waiting lock (busy spinning) */
class SpinLock
{
public:
    /** Implements RAII for the spin lock class */
    struct Guard
    {
        Guard(SpinLock &s) : spin(s) { spin.lock(); }
        ~Guard() { spin.unlock(); }
        SpinLock &spin;
    };

    SpinLock() : lock_(false) {}
    ~SpinLock() {}

    /** acquires the lock */
    void lock() noexcept
    {
        for (;;)
        {
            if (!lock_.exchange(true, std::memory_order_acquire))
            {
                return;
            }
            while (lock_.load(std::memory_order_relaxed))
            {
                __builtin_ia32_pause();
            }
        }
    }

    /** tries to acquire the lock */
    bool try_lock() noexcept
    {
        return !lock_.load(std::memory_order_relaxed) &&
               !lock_.exchange(true, std::memory_order_acquire);
    }

    /** Releases the lock */
    void unlock() noexcept
    {
        lock_.store(false, std::memory_order_release);
    }

    /** Returns lock status */
    bool locked() noexcept
    {
        return lock_.load(std::memory_order_relaxed);
    }

private:
    std::atomic<bool> lock_;
};
