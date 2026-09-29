#pragma once


#include <memory>
#include <mutex>
#include <string>
#include <vector>

class ILoggerSink {
public:
    virtual ~ILoggerSink() = default;
    virtual void log(const std::string& level, const std::string& message) = 0;
};

class Logger {
public:
    static Logger& getInstance();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void addSink(std::unique_ptr<ILoggerSink> sink);
    void log(const std::string& level, const std::string& message) const;
    void info(const std::string& message) const;
    void warn(const std::string& message) const;
    void error(const std::string& message) const;
    Logger() = default;

private:
    

    static std::unique_ptr<Logger> instance_;
    mutable std::mutex mutex_;
    std::vector<std::unique_ptr<ILoggerSink>> sinks_;
};
