import argparse
import socket
import subprocess
import threading
from concurrent.futures import Future, ThreadPoolExecutor
from typing import Optional


class ProcessPool:
    """Singleton-style process pool. Python has no unique_ptr, so we keep a single shared executor instance."""

    _instance: Optional["ProcessPool"] = None
    _lock = threading.Lock()

    def __new__(cls, max_workers: int = 4):
        if cls._instance is None:
            with cls._lock:
                if cls._instance is None:
                    cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self, max_workers: int = 4):
        if getattr(self, "_initialized", False):
            return
        self._initialized = True
        self.max_workers = max_workers
        self._executor = ThreadPoolExecutor(max_workers=max_workers)

    def submit(self, command: str, cwd: Optional[str] = None) -> Future:
        return self._executor.submit(self._run_process, command, cwd)

    @staticmethod
    def _run_process(command: str, cwd: Optional[str] = None):
        proc = subprocess.run(
            command,
            shell=True,
            cwd=cwd,
            capture_output=True,
            text=True,
        )
        return {
            "returncode": proc.returncode,
            "stdout": proc.stdout,
            "stderr": proc.stderr,
            "command": command,
        }

    def shutdown(self):
        self._executor.shutdown(wait=True)


class ThreadedTCPServer:
    def __init__(self, host: str = "127.0.0.1", port: int = 5000, workers: int = 4):
        self.host = host
        self.port = port
        self.process_pool = ProcessPool(max_workers=workers)

    def start(self):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind((self.host, self.port))
        server_socket.listen(5)

        print(f"Thread pool server listening on {self.host}:{self.port}")

        try:
            while True:
                client_socket, address = server_socket.accept()
                print(f"Client connected: {address}")
                self._handle_client(client_socket)
        finally:
            server_socket.close()

    def _handle_client(self, client_socket: socket.socket):
        try:
            data = client_socket.recv(4096)
            if not data:
                return

            message = data.decode("utf-8", errors="ignore").strip()
            print(f"Received: {message}")

            future = self.process_pool.submit(message)
            result = future.result(timeout=30)

            response = (
                f"returncode={result['returncode']}\n"
                f"stdout={result['stdout']}\n"
                f"stderr={result['stderr']}"
            )
            client_socket.sendall(response.encode("utf-8"))
        except Exception as exc:  # pragma: no cover - runtime guard
            client_socket.sendall(f"ERROR: {exc}".encode("utf-8"))
        finally:
            client_socket.close()


def parse_args():
    parser = argparse.ArgumentParser(description="Run a threaded process pool server.")
    parser.add_argument("--host", default="127.0.0.1", help="Address to bind the server to.")
    parser.add_argument("--port", type=int, default=5000, help="Port to bind the server to.")
    parser.add_argument("--workers", type=int, default=4, help="Number of worker threads in the pool.")
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    server = ThreadedTCPServer(host=args.host, port=args.port, workers=args.workers)
    server.start()
