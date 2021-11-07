#pragma once
#include <cstdint>
#include <memory>
#include <atomic>

/** Holds a work buffer, its metadata and synchonization elements */
struct Buffer
{
    char *data;                 //! The allocated data
    uint32_t size;              //! Buffer capacity
    uint32_t used;              //! Used amount so far
    uint32_t index;             //! Indicates which thread owns this buffer
    std::atomic<uint32_t> flag; //! To synchronize between this owner and the main thread
};
using BufferPtr = std::shared_ptr<Buffer>;
