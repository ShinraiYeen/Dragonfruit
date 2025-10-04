#include "dragonfruit_engine/core/buffer.hpp"

#include <mutex>
#include <optional>
#include <utility>

#include "dragonfruit_engine/logging/logger.hpp"

namespace dragonfruit {
Buffer::Buffer(size_t capacity) : m_capacity(capacity) {
    Logger::Get()->debug("Initialized buffer with capacity {}", capacity);
}

void Buffer::Push(std::vector<uint8_t> data) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond_producer.wait(lock, [&] { return m_queue.size() < m_capacity || m_abort; });

    if (m_abort) return;

    m_queue.push(data);
}

std::optional<std::vector<uint8_t>> Buffer::Pop() {
    std::scoped_lock<std::mutex> lock(m_mutex);

    // Immediately return an empty object if there is nothing to read from the internal queue.
    if (m_queue.empty()) {
        m_cond_producer.notify_one();
        return std::nullopt;
    }

    std::vector<uint8_t> data = std::move(m_queue.front());
    m_queue.pop();

    // Notify any producers that there is now space in the queue to push more data
    m_cond_producer.notify_one();

    return data;
}

void Buffer::Clear() {
    std::scoped_lock<std::mutex> lock(m_mutex);
    std::queue<std::vector<uint8_t>> empty;
    std::swap(m_queue, empty);

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
