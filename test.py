import socket
import threading
import random
import string

# all of this is mostly chatgpt; used just for stress testing the code in a quick manner.


def random_string(min_length=5, max_length=15):
    """
    Generate a random string with length between min_length and max_length.

    Parameters:
    - min_length (int): Minimum length of the string (default: 5)
    - max_length (int): Maximum length of the string (default: 15)

    Returns:
    - str: Random string containing uppercase, lowercase letters and digits
    """
    # Validate input
    if min_length < 1:
        raise ValueError("Minimum length must be at least 1")
    if max_length < min_length:
        raise ValueError(
            "Maximum length must be greater than or equal to minimum length"
        )

    # Randomly choose a length between min and max
    length = random.randint(min_length, max_length)

    # Generate random string
    characters = (
        string.ascii_letters + string.digits
    )  # You can add string.punctuation for symbols
    return "".join(random.choice(characters) for _ in range(length))


def client_thread(id):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(("127.0.0.1", 8080))
        # Receive server message
        msg = s.recv(512)
        print(f"Client {id} received: {msg.decode()}")
        # Send message to server (do later...)
        # s.sendall(random_string(10, 20).encode())
        # s.close()
    except Exception as e:
        print(f"Client {id} error: {e}")


threads = []
NUM_CONNECTIONS = 10

for i in range(NUM_CONNECTIONS):
    t = threading.Thread(target=client_thread, args=(i,))
    t.start()
    threads.append(t)

for t in threads:
    t.join()

print("All client threads have finished.")
