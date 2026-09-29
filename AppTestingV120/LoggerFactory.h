#pragma once

#include "Logger.h"

#include <memory>
#include <string>

class LoggerFactory {
public:
    static std::unique_ptr<ILoggerSink> createConsoleSink();
    static std::unique_ptr<ILoggerSink> createFileSink(const std::string& path);
    static std::unique_ptr<ILoggerSink> createNetworkSink(const std::string& host, int port);
};
