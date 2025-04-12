import socket
import threading


def client_thread(id):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(("127.0.0.1", 8080))
        # Receive server message
        msg = s.recv(1024)
        print(f"Client {id} received: {msg.decode()}")
        # Send message to server
        s.sendall("Hello from client!".encode())
        s.close()
    except Exception as e:
        print(f"Client {id} error: {e}")


threads = []
NUM_CONNECTIONS = 5

for i in range(NUM_CONNECTIONS):
    t = threading.Thread(target=client_thread, args=(i,))
    t.start()
    threads.append(t)

for t in threads:
    t.join()

print("All client threads have finished.")
