#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "LoggerFactory.h"

#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {

    class ConsoleSink final : public ILoggerSink {
    public:
        void log(const std::string& level, const std::string& message) override {
            std::cout << message << std::endl;
            (void)level;
        }
    };

    class FileSink final : public ILoggerSink {
    public:
        explicit FileSink(std::string path) : path_(std::move(path)) {}

        void log(const std::string& level, const std::string& message) override {
            std::ofstream output(path_, std::ios::app);
            if (output.is_open()) {
                output << message << '\n';
            }
            (void)level;
        }

    private:
        std::string path_;
    };

    class NetworkSink final : public ILoggerSink {
    public:
        NetworkSink(std::string host, int port) : host_(std::move(host)), port_(port) {}

        void log(const std::string& level, const std::string& message) override {
            const std::string payload = message;

#ifdef _WIN32
            WSADATA wsaData{};
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                return;
            }

            SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (clientSocket == INVALID_SOCKET) {
                return;
            }

            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_port = htons(static_cast<u_short>(port_));
            address.sin_addr.s_addr = inet_addr(host_.c_str());

            if (connect(clientSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
                closesocket(clientSocket);
                WSACleanup();
                return;
            }

            send(clientSocket, payload.c_str(), static_cast<int>(payload.size()), 0);
            closesocket(clientSocket);
            WSACleanup();
#else
            int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (clientSocket < 0) {
                return;
            }

            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_port = htons(port_);
            inet_pton(AF_INET, host_.c_str(), &address.sin_addr);

            if (connect(clientSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
                close(clientSocket);
                return;
            }

            send(clientSocket, payload.c_str(), payload.size(), 0);
            close(clientSocket);
#endif

            (void)level;
        }

    private:
        std::string host_;
        int port_;
    };

} // namespace

std::unique_ptr<ILoggerSink> LoggerFactory::createConsoleSink() {
    return std::make_unique<ConsoleSink>();
}

std::unique_ptr<ILoggerSink> LoggerFactory::createFileSink(const std::string& path) {
    return std::make_unique<FileSink>(path);
}

std::unique_ptr<ILoggerSink> LoggerFactory::createNetworkSink(const std::string& host, int port) {
    return std::make_unique<NetworkSink>(host, port);
}
