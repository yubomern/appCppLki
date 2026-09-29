import argparse
import socket


class TCPServer:
    def __init__(self, host="127.0.0.1", port=5000):
        self.host = host
        self.port = port

    def start(self):
        server_socket = socket.socket(
            socket.AF_INET,
            socket.SOCK_STREAM
        )

        server_socket.setsockopt(
            socket.SOL_SOCKET,
            socket.SO_REUSEADDR,
            1
        )

        server_socket.bind((self.host, self.port))
        server_socket.listen(5)

        print(f"Server listening on {self.host}:{self.port}")

        while True:
            client_socket, address = server_socket.accept()

            print(f"Client connected: {address}")

            data = client_socket.recv(1024)

            if data:
                message = data.decode("utf-8")

                print(f"Received: {message}")

                response = f"Python received: {message}"

                client_socket.sendall(
                    response.encode("utf-8")
                )

            client_socket.close()


def parse_args():
    parser = argparse.ArgumentParser(description="Run the TCP server.")
    parser.add_argument("--host", default="127.0.0.1", help="Host address to bind the server to.")
    parser.add_argument("--port", type=int, default=5000, help="Port number to bind the server to.")
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    server = TCPServer(host=args.host, port=args.port)
    server.start()
