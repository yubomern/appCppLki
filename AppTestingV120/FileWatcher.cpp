#include "FileWatcher.h"

#include <chrono>
#include <iostream>
#include <thread>

FileWatcher::FileWatcher(std::string path) : path_(std::move(path)) {}

FileWatcher::~FileWatcher() {
    stop();
}

void FileWatcher::start() {
    if (running_.load()) {
        return;
    }

    running_.store(true);
    worker_ = std::thread(&FileWatcher::run, this);
}

void FileWatcher::stop() {
    running_.store(false);
    if (worker_.joinable()) {
        worker_.join();
    }
}

void FileWatcher::run() {
    std::error_code ec;
    if (std::filesystem::exists(path_, ec)) {
        lastWrite_ = std::filesystem::last_write_time(path_, ec);
    }

    while (running_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::error_code statusCode;
        if (!std::filesystem::exists(path_, statusCode)) {
            std::cout << "Watcher: file not found: " << path_ << std::endl;
            continue;
        }

        const auto currentWrite = std::filesystem::last_write_time(path_, statusCode);
        if (statusCode) {
            continue;
        }

        if (currentWrite != lastWrite_) {
            lastWrite_ = currentWrite;
            std::cout << "Watcher: file changed: " << path_ << std::endl;
        }
    }
}
