// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_READERWRITERQUEUE_H_
#define _DBM_READERWRITERQUEUE_H_

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
class ReaderWriterQueue
{
private:
    std::queue<std::unique_ptr<T>> queue;
    mutable std::mutex mutex;
    std::condition_variable condition;

public:
    // Write (producer) - non-blocking
    void push(std::unique_ptr<T> item)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(std::move(item));
        }
        condition.notify_one();
    }

    // Read (consumer) - non-blocking
    // Returns nullptr if queue is empty
    std::unique_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (queue.empty())
            return nullptr;

        auto item = std::move(queue.front());
        queue.pop();
        return item;
    }

    // Utility functions
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }
};

#endif /* _DBM_READERWRITERQUEUE_H_ */
