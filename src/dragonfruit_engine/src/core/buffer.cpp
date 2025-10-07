#include "dragonfruit_engine/core/buffer.hpp"

#include <mutex>

#include "dragonfruit_engine/logging/logger.hpp"

namespace dragonfruit {
Buffer::Buffer(size_t capacity) : m_buffer(capacity), m_capacity(capacity) {
    Logger::Get()->debug("Initialized buffer with capacity {}", capacity);
}

void Buffer::Push(std::vector<uint8_t> data) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond_producer.wait(lock, [&] { return m_size + data.size() < m_capacity || m_abort; });

    if (m_abort) return;

    for (size_t i = 0; i < data.size(); i++) {
        m_buffer[m_tail] = data[i];
        m_tail = (m_tail + 1) % m_capacity;
    }
    m_size += data.size();
}

std::vector<uint8_t> Buffer::Pop(size_t len) {
    std::scoped_lock<std::mutex> lock(m_mutex);

    size_t to_read = std::min(m_size, len);
    std::vector<uint8_t> data(to_read);
    for (size_t i = 0; i < data.size(); i++) {
        data[i] = m_buffer[m_head];
        m_head = (m_head + 1) % m_capacity;
    }
    m_size -= data.size();

    // Notify any producers that there is now space in the queue to push more data
    m_cond_producer.notify_one();

    return data;
}

void Buffer::Clear() {
    std::scoped_lock<std::mutex> lock(m_mutex);
    m_size = 0;
    m_head = 0;
    m_tail = 0;

    m_cond_producer.notify_all();
}

void Buffer::Abort(bool abort) {
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_abort = abort;
    }
    m_cond_producer.notify_all();
}

}  // namespace dragonfruit
