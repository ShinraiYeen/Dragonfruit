#include "dragonfruit_engine/core/buffer.hpp"

namespace dragonfruit {
Buffer::Buffer(size_t capacity) : m_capacity(capacity) {}

void Buffer::Push(std::vector<uint8_t> data) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond_producer.wait(lock, [&] { return m_queue.size() < m_capacity; });

    m_queue.push(std::move(data));
    lock.unlock();
    m_cond_consumer.notify_one();
}

std::vector<uint8_t> Buffer::Pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond_consumer.wait(lock, [&] { return !m_queue.empty(); });

    std::vector<uint8_t> data = std::move(m_queue.front());
    m_queue.pop();
    lock.unlock();
    m_cond_producer.notify_one();

    return data;
}

}  // namespace dragonfruit