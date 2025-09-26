#pragma once

#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>

#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"

#ifdef ENABLE_DEBUG_LOGGING
constexpr auto LOG_LEVEL = spdlog::level::debug;
#else
constexpr auto LOG_LEVEL = spdlog::level::info;
#endif

namespace dragonfruit {
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

        auto t = std::time(nullptr);
        std::tm tm{};
        localtime_r(&t, &tm);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
        std::string filename = "dragonfruit_" + oss.str() + ".log";

        fs::path log_file = log_dir / filename;

        auto logger = spdlog::basic_logger_mt("file_logger", log_file.string(), true);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
        logger->set_level(LOG_LEVEL);

        return logger;
    }
};
}  // namespace dragonfruit
