#include "Client.h"
#include "FileWatcher.h"
#include "Logger.h"
#include "LoggerFactory.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <iostream>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <mutex>
#include <string>

#include <future>
int mainLmm(const std::string & host= "127.0.0.1",short  port =5000)
{
    SocketApp server;

    server.Run(
      host ,
        port 
    );

    return 0;
}
#include "ThreadPool.h"

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 9000;
    std::string logPath = "app.log";
    ThreadPool pool(6);
    auto future1 = pool.enqueue([](int a, int b) { return a + b; }, 10, 20);
    auto future2 = pool.enqueue([](const std::string& s) { return s + " from thread pool"; }, std::string("hello"));

    pool.enqueue(mainLmm, "127.0.0.1",6000);
    std::cout << "future1: " << future1.get() << '\n';
    std::cout << "future2: " << future2.get() << '\n';

    auto future3 = pool.enqueue([] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 42;
        });

    std::cout << "future3: " << future3.get() << '\n';

    std::future<int> ml_ = std::async(std::launch::async, mainLmm, "127.0.0.1",5001);
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        }
        else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
        else if (arg == "--log" && i + 1 < argc) {
            logPath = argv[++i];
        }
    }

    auto& logger = Logger::getInstance();
    logger.addSink(LoggerFactory::createConsoleSink());
    logger.addSink(LoggerFactory::createFileSink(logPath));

    logger.info("Logger initialized");
    logger.warn("Watcher started");
    ml_.get();
    std::filesystem::path watchPath = logPath;
    FileWatcher watcher(watchPath.string());
    watcher.start();

    std::cout << "Listening on " << host << ":" << port << " and watching " << logPath << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));
    watcher.stop();

    logger.info("Application shutdown");
    return 0;
}
