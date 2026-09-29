#pragma once

#include <atomic>
#include <filesystem>
#include <string>
#include <thread>

class FileWatcher {
public:
    explicit FileWatcher(std::string path);
    ~FileWatcher();

    void start();
    void stop();

private:
    void run();

    std::string path_;
    std::atomic<bool> running_{ false };
    std::thread worker_;
    std::filesystem::file_time_type lastWrite_{};
};
