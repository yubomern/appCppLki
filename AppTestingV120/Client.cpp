#include "Client.h"

#include <iostream>

#pragma comment(lib, "Ws2_32.lib")


SocketApp::SocketApp()
    : server_fd(INVALID_SOCKET),
    client_fd(INVALID_SOCKET),
    server_sock{},
    cli_sock{},
    wSdata{},
    res_(0),
    winsockInitialized(false)
{
}


SocketApp::~SocketApp()
{
    Cleanup();
}


// --------------------------------------------------
// Initialize Winsock
// --------------------------------------------------

bool SocketApp::Initialize()
{
    if (winsockInitialized)
    {
        return true;
    }

    res_ = WSAStartup(
        MAKEWORD(2, 2),
        &wSdata
    );


    if (res_ != 0)
    {
        std::cerr
            << "WSAStartup failed: "
            << res_
            << '\n';

        return false;
    }


    winsockInitialized = true;

    std::cout << "[OK] Winsock initialized\n";

    return true;
}


// --------------------------------------------------
// Create server socket
// --------------------------------------------------

bool SocketApp::CreateServerSocket()
{
    server_fd = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );


    if (server_fd == INVALID_SOCKET)
    {
        std::cerr
            << "socket() failed: "
            << WSAGetLastError()
            << '\n';

        return false;
    }


    std::cout << "[OK] Server socket created\n";

    return true;
}


// --------------------------------------------------
// Bind
// --------------------------------------------------

bool SocketApp::Bind(
    const std::string& ip,
    unsigned short port
)
{
    server_sock = {};

    server_sock.sin_family = AF_INET;

    server_sock.sin_port = htons(port);


    res_ = inet_pton(
        AF_INET,
        ip.c_str(),
        &server_sock.sin_addr
    );


    if (res_ != 1)
    {
        std::cerr
            << "inet_pton() failed\n";

        return false;
    }


    res_ = bind(
        server_fd,
        reinterpret_cast<sockaddr*>(
            &server_sock
            ),
        sizeof(server_sock)
        );


    if (res_ == SOCKET_ERROR)
    {
        std::cerr
             << "bind() failed: "
             << WSAGetLastError()
            << '\n';

        return false;
    }


        std::cout
            << "[OK] *ind: "
            << ip
            << ":"
            << port
            << '\n';
    
        return true;
        }


        // ---------*----------------------------------*-----
        // Listen
        // ---------------*----------------------------------*
        bool SocketApp::Listen(int backlog)
    {
        res_ = listen(
            server_fd,
            backlog
        );


        if (res_ == SOCKET_ERROR)
        {
            std::cerr
                << "li*ten() failed: "
                << WSAGetLastError()
                << '\n';

            return false;
        }


        std::cout << "[OK] Server listening\n";

        return true;
    }


    // --------------------------------------------------
    // Accept
    // --------------------------------------------------

    bool SocketApp::Accept()
    {
        int clientSize =
            sizeof(cli_sock);


        std::cout
            << "[WAIT] Waiting for client...\n";


        client_fd = accept(
            server_fd,
            reinterpret_cast<sockaddr*>(
                &cli_sock
                ),
            &clientSize
        );


        if (client_fd == INVALID_SOCKET)
        {
            std::cerr
                << "accept() failed: "
                << WSAGetLastError()
                << '\n';

            return false;
        }


        char clientIp[INET_ADDRSTRLEN]{};


        inet_ntop(
            AF_INET,
            &cli_sock.sin_addr,
            clientIp,
            sizeof(clientIp)
        );


        std::cout
            << "[CLIENT] Connected: "
            << clientIp
            << ":"
            << ntohs(cli_sock.sin_port)
            << '\n';


        return true;
    }


    // --------------------------------------------------
    // Send
    // --------------------------------------------------

    bool SocketApp::Send(
        const std::string& message
    )
    {
        if (client_fd == INVALID_SOCKET)
        {
            std::cerr
                << "No connected client\n";

            return false;
        }


        const char* buffer =
            message.c_str();


        int totalBytes = 0;

        int messageSize =
            static_cast<int>(
                message.size()
                );


        while (totalBytes < messageSize)
        {
            int bytesSent = send(
                client_fd,
                buffer + totalBytes,
                messageSize - totalBytes,
                0
            );


            if (bytesSent == SOCKET_ERROR)
            {
                std::cerr
                    << "send() failed: "
                    << WSAGetLastError()
                    << '\n';

                return false;
            }


            totalBytes += bytesSent;
        }


        return true;
    }


    // --------------------------------------------------
    // Receive
    // --------------------------------------------------

    std::string SocketApp::Receive()
    {
        if (client_fd == INVALID_SOCKET)
        {
            return {};
        }


        std::array<char, 4096> buffer{};


        int received = recv(
            client_fd,
            buffer.data(),
            static_cast<int>(
                buffer.size() - 1
                ),
            0
        );


        if (received == 0)
        {
            std::cout
                << "[CLIENT] Connection closed\n";

            return {};
        }


        if (received == SOCKET_ERROR)
        {
            std::cerr
                << "recv() failed: "
                << WSAGetLastError()
                << '\n';

            return {};
        }


        return std::string(
            buffer.data(),
            received
        );
    }


    // --------------------------------------------------
    // Close Client
    // --------------------------------------------------

    void SocketApp::CloseClient()
    {
        if (client_fd != INVALID_SOCKET)
        {
            shutdown(
                client_fd,
                SD_BOTH
            );

            closesocket(client_fd);

            client_fd = INVALID_SOCKET;


            std::cout
                << "[CLIENT] Socket closed\n";
        }
    }


    // --------------------------------------------------
    // Close Server
    // --------------------------------------------------

    void SocketApp::CloseServer()
    {
        if (server_fd != INVALID_SOCKET)
        {
            closesocket(server_fd);

            server_fd = INVALID_SOCKET;
        }
    }


    // --------------------------------------------------
    // Cleanup
    //

    void _Cleanup() {
        WSACleanup();
}

    void SocketApp::Cleanup()
    {
        if (client_fd != INVALID_SOCKET)
        {
            closesocket(client_fd);
            client_fd = INVALID_SOCKET;
        }

        if (server_fd != INVALID_SOCKET)
        {
            closesocket(server_fd);
            server_fd = INVALID_SOCKET;
        }

        WSACleanup();
    }
    void SocketApp::Run(
        const std::string& ip,
        unsigned short port
    )
    {
        // 1. Initialize Winsock
        if (!Initialize())
        {
            std::cerr << "[ERROR] Initialize() failed\n";
            return;
        }

        // 2. Create TCP socket
        if (!CreateServerSocket())
        {
            std::cerr << "[ERROR] CreateServerSocket() failed\n";
            return;
        }

        // 3. Bind socket to IP + port
        if (!Bind(ip, port))
        {
            std::cerr << "[ERROR] Bind() failed\n";
            return;
        }

        // 4. Start listening
        if (!Listen())
        {
            std::cerr << "[ERROR] Listen() failed\n";
            return;
        }

        std::cout
            << "=============================\n"
            << "      C++ TCP SERVER\n"
            << "=============================\n"
            << "IP   : " << ip << '\n'
            << "PORT : " << port << '\n'
            << "=============================\n";


        // Server loop
        while (true)
        {
            std::cout << "\n[SERVER] Waiting for client...\n";

            // 5. Accept client
            if (!Accept())
            {
                std::cerr << "[ERROR] Accept() failed\n";
                continue;
            }

            std::cout << "[SERVER] Client connected\n";


            // Communication loop for current client
            while (true)
            {
                // 6. Receive message
                std::string message = Receive();

                if (message.empty())
                {
                    std::cout
                        << "[SERVER] Client disconnected\n";

                    break;
                }


                std::cout
                    << "[CLIENT] "
                    << message
                    << '\n';


                // 7. Commands
                if (message == "PING")
                {
                    Send("PONG");
                }

                else if (message == "HELLO")
                {
                    Send("HELLO FROM C++");
                }

                else if (message == "EXIT")
                {
                    Send("BYE");

                    std::cout
                        << "[SERVER] Client requested EXIT\n";

                    break;
                }

                else
                {
                    // Echo response
                    std::string response =
                        "C++ RECEIVED: " + message;

                    Send(response);
                }
            }


            // 8. Close current client
            CloseClient();
        }
    }
