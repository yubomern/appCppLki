#include "Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

std::unique_ptr<Logger> Logger::instance_;

Logger& Logger::getInstance() {
    if (!instance_) {
        instance_ = std::make_unique<Logger>();
    }
    return *instance_;
}

void Logger::addSink(std::unique_ptr<ILoggerSink> sink) {
    if (sink) {
        sinks_.push_back(std::move(sink));
    }
}

void Logger::log(const std::string& level, const std::string& message) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    std::ostringstream stream;
    stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    const std::string stamp = stream.str();
    const std::string formatted = "[" + stamp + "] [" + level + "] " + message;

    for (const auto& sink : sinks_) {
        sink->log(level, formatted);
    }
}

void Logger::info(const std::string& message) const {
    log("INFO", message);
}

void Logger::warn(const std::string& message) const {
    log("WARN", message);
}

void Logger::error(const std::string& message) const {
    log("ERROR", message);
}
