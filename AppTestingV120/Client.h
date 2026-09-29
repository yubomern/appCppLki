#pragma once

#include "Include.h"

#include <array>
#include <string>


struct Data
{
    std::string datatr;
    std::string dataReCV;

    int _s = 0;

    std::array<std::string, 3> result{};
};


class SocketApp
{
private:

    SOCKET server_fd;
    SOCKET client_fd;

    sockaddr_in server_sock;
    sockaddr_in cli_sock;

    WSADATA wSdata{};

    int res_;

    bool winsockInitialized;


public:

    // Constructor / Destructor
    SocketApp();

    ~SocketApp();


    // Prevent copy
    SocketApp(const SocketApp&) = delete;

    SocketApp& operator=(const SocketApp&) = delete;


    // Socket initialization
    bool Initialize();

    bool CreateServerSocket();

    bool Bind(
        const std::string& ip,
        unsigned short port
    );

    bool Listen(int backlog = 5);

    bool Accept();


    // Communication
    bool Send(const std::string& message);

    std::string Receive();


    // Connection management
    void CloseClient();

    void CloseServer();

    void Cleanup();


    // Main server function
    void Run(
        const std::string& ip = "127.0.0.1",
        unsigned short port = 5000
    );


    // Getters
    SOCKET GetServerSocket() const;

    SOCKET GetClientSocket() const;
};