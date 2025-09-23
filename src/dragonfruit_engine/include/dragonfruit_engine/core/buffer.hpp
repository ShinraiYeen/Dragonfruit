#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

namespace dragonfruit {

enum class ItemType {
    DecodedFrame,
    DecodeFinished,
};

struct BufferItem {
    BufferItem(ItemType item_type) : item_type(item_type) {}
    BufferItem(ItemType item_type, std::vector<uint8_t> data) : item_type(item_type), data(std::move(data)) {}

    ItemType item_type;
    std::vector<uint8_t> data;
};

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
    void Push(BufferItem item);

    /**
     * @brief This call is non-blocking. Pops the next value from the front of the queue. If there are no items in the
     * queue, std::nullopt will be returned. This must be handled appropriately.
     */
    std::optional<BufferItem> Pop();

    /**
     * @brief Clears the buffer entirely, removing all data from the internal queue.
     */
    void Clear();

    void SignalShutdown(bool shutdown);

   private:
    size_t m_capacity;
    std::mutex m_mutex;
    std::condition_variable m_cond_producer;
    std::queue<BufferItem> m_queue;
    bool m_stop = false;
};
}  // namespace dragonfruit
