#pragma once

#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>

#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/rotating_file_sink.h"

namespace dragonfruit {

#ifdef ENABLE_DEBUG_LOGGING
constexpr auto LOG_LEVEL = spdlog::level::debug;
#else
constexpr auto LOG_LEVEL = spdlog::level::info;
#endif
constexpr size_t DRAGONFRUIT_MAX_LOG_SIZE = 1024 * 1024 * 5;
constexpr size_t DRAGONFRUIT_MAX_LOG_FILES = 10;

class Logger {
   public:
    static std::shared_ptr<spdlog::logger> Get() {
        static std::shared_ptr<spdlog::logger> instance = CreateLogger();
        return instance;
    }

   private:
    static std::shared_ptr<spdlog::logger> CreateLogger() {
        namespace fs = std::filesystem;

        std::string home = std::getenv("HOME") ? std::getenv("HOME") : ".";
        fs::path log_dir = fs::path(home) / ".local" / "share" / "dragonfruit" / "logs";

        fs::create_directories(log_dir);

        fs::path log_file = log_dir / "dragonfruit.log";

        auto logger =
            spdlog::rotating_logger_mt("file-logger", log_file, DRAGONFRUIT_MAX_LOG_SIZE, DRAGONFRUIT_MAX_LOG_FILES);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
        logger->set_level(LOG_LEVEL);
        logger->flush_on(LOG_LEVEL);

        return logger;
    }
};
}  // namespace dragonfruit
