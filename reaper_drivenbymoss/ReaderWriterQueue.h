// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_READERWRITERQUEUE_H_
#define _DBM_READERWRITERQUEUE_H_

#include <atomic>
#include <cstddef>
#include <memory>
#include <cassert>
#include <type_traits>


// SPSC bounded queue: capacity must be a power of two
template<typename T, size_t Capacity>
class ReaderWriterQueue
{
    static_assert((Capacity& (Capacity - 1)) == 0, "Capacity must be a power of two");
public:
    ReaderWriterQueue() noexcept
        : head_(0), tail_(0)
    {
        static_assert(std::is_nothrow_constructible<T>::value || std::is_trivially_destructible<T>::value, "T should be cheaply movable or trivially destructible for the RT path");
        for (size_t i = 0; i < Capacity; ++i) buffer_[i] = nullptr;
    }

    ReaderWriterQueue(const ReaderWriterQueue&) = delete;
    ReaderWriterQueue& operator=(const ReaderWriterQueue&) = delete;

    // Non-blocking push: returns true on success, false if the queue is full.
    bool push(std::unique_ptr<T> item) noexcept
    {
        T* raw = item.release(); // transfer ownership into the queue
        const size_t t = tail_.load(std::memory_order_relaxed);
        const size_t next = (t + 1) & mask();
        if (next == head_.load(std::memory_order_acquire))
        {
            // queue full
            // restore ownership for caller (avoid leaking)
            item.reset(raw);
            return false;
        }

        DISABLE_WARNING_USE_GSL_AT
        DISABLE_WARNING_ACCESS_ARRAYS_WITH_CONST
        buffer_[t] = raw;

        // publish the new tail (make the write visible to consumer)
        tail_.store(next, std::memory_order_release);
        return true;
    }

    // Non-blocking pop: returns nullptr if empty.
    std::unique_ptr<T> pop() noexcept
    {
        const size_t h = head_.load(std::memory_order_relaxed);
        if (h == tail_.load(std::memory_order_acquire))
            return nullptr;

        T* raw = buffer_[h];
        // Not strictly required but helps debugging
        buffer_[h] = nullptr;

        const size_t next = (h + 1) & mask();
        head_.store(next, std::memory_order_release);

        return std::unique_ptr<T>(raw);
    }

    // Optional: check approximate size (non-atomic consistent read is not guaranteed constant-time with concurrent ops)
    size_t unsafe_size() const noexcept
    {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);
        if (t >= h) return t - h;
        return Capacity - (h - t);
    }

    bool empty() const noexcept { return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire); }
    bool full()  const noexcept { return ((tail_.load(std::memory_order_acquire) + 1) & mask()) == head_.load(std::memory_order_acquire); }

private:
    static constexpr size_t mask() noexcept
    {
        return Capacity - 1;
    }

    // buffer stores raw pointers, ownership semantics are handled by push/pop
    T* buffer_[Capacity];
    // head = pop index, tail = push index. Only producer touches tail_, only consumer touches head_.
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

#endif /* _DBM_READERWRITERQUEUE_H_ */
