#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>

namespace dragonfruit {

/**
 * @brief A thread-safe, producer blocking, consumer non-blocking ring-buffer implementation for sharing PCM samples.
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
    std::vector<uint8_t> Pop(size_t len);

    /**
     * @brief Clears the buffer entirely, removing all data from the internal queue.
     */
    void Clear();

    /**
     * @brief Signals all producers/consumers to stop blocking and return without having done any work.
     */
    void Abort(bool abort);

   private:
    std::mutex m_mutex;
    std::condition_variable m_cond_producer;
    std::vector<uint8_t> m_buffer;
    size_t m_head = 0;      // Read head
    size_t m_tail = 0;      // Write head
    size_t m_size = 0;      // Current buffer fill size
    size_t m_capacity = 0;  // Buffer capacity
    bool m_abort = false;
};
}  // namespace dragonfruit
