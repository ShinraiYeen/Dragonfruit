#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

namespace dragonfruit {

/**
 * @brief A thread-safe, producer blocking, consumer non-blocking queue.
 */
class Buffer {
   public:
    Buffer(size_t capacity);

    /**
     * @brief This is a blocking call. Pushes an item to the internal queue, blocking until there is space on the queue
     * available or until signaled to return.
     *
     * @param item The buffer item containing data to push to the queue.
     */
    void Push(std::vector<uint8_t> data);

    /**
     * @brief This call is non-blocking. Pops the next value from the front of the queue. If there are no items in the
     * queue, std::nullopt will be returned. This must be handled appropriately.
     */
    std::optional<std::vector<uint8_t>> Pop();

    /**
     * @brief Clears the buffer entirely, removing all data from the internal queue.
     */
    void Clear();

    /**
     * @brief Signals all producers/consumers to stop blocking and return without having done any work.
     */
    void Abort(bool abort);

   private:
    size_t m_capacity;
    std::mutex m_mutex;
    std::condition_variable m_cond_producer;
    std::queue<std::vector<uint8_t>> m_queue;
    bool m_abort = false;
};
}  // namespace dragonfruit
