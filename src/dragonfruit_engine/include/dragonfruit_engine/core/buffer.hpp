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
     * @brief Pushes a chunk of data to the buffer. This will block until there is enough space available in the buffer.
     *
     * @param data Data to push to the buffer.
     */
    void Push(std::vector<uint8_t> data);

    /**
     * @brief Pops a maximum of <len> bytes from the buffer. If there is less data than that available in the buffer,
     * only the available data will be returned. This is a non-blocking call.
     *
     * @param len The maximum number of bytes to pop from the buffer.
     */
    std::vector<uint8_t> Pop(size_t len);

    /**
     * @brief Clears the buffer entirely, removing all data from the internal buffer.
     */
    void Clear();

    /**
     * @brief Signals all producers/consumers to stop blocking and return without having done any work.
     *
     * @param abort Aborts the buffer if set to true. If set to false, the producer will be signaled and continue work.
     */
    void Abort(bool abort);

   private:
    std::mutex m_mutex;
    std::condition_variable m_cond_producer;
    bool m_abort = false;

    std::vector<uint8_t> m_buffer;
    size_t m_head = 0;      // Read head
    size_t m_tail = 0;      // Write head
    size_t m_size = 0;      // Current buffer fill size
    size_t m_capacity = 0;  // Buffer capacity
};
}  // namespace dragonfruit
