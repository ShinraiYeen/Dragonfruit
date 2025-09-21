#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

namespace dragonfruit {

class Buffer {
   public:
    Buffer(size_t capacity);

    void Push(std::vector<uint8_t> data);
    std::optional<std::vector<uint8_t>> Pop();

    void Shutdown();
    void Reset();

   private:
    size_t m_capacity;
    std::mutex m_mutex;
    std::condition_variable m_cond_producer;
    std::condition_variable m_cond_consumer;
    std::queue<std::vector<uint8_t>> m_queue;
    bool m_stop = false;
};
}  // namespace dragonfruit
