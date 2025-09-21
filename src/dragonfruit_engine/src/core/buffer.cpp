#include "dragonfruit_engine/core/buffer.hpp"

#include <mutex>
#include <optional>

namespace dragonfruit {
Buffer::Buffer(size_t capacity) : m_capacity(capacity) {}

void Buffer::Push(std::vector<uint8_t> data) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond_producer.wait(lock, [&] { return m_queue.size() < m_capacity || m_stop; });

    if (m_stop) return;

    m_queue.push(std::move(data));
    lock.unlock();
    m_cond_consumer.notify_one();
}

std::optional<std::vector<uint8_t>> Buffer::Pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond_consumer.wait(lock, [&] { return !m_queue.empty() || m_stop; });

    if (m_stop) return std::nullopt;

    std::vector<uint8_t> data = std::move(m_queue.front());
    m_queue.pop();
    lock.unlock();
    m_cond_producer.notify_one();

    return data;
}

void Buffer::Reset() {
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_stop = true;
    }

    m_cond_producer.notify_all();
    m_cond_consumer.notify_all();

    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_stop = false;
    }
}

void Buffer::Shutdown() {
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_cond_consumer.notify_all();
    m_cond_producer.notify_all();
}

}  // namespace dragonfruit
