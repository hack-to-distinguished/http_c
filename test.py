# all of this is mostly chatgpt; used just for stress testing the code in a quick manner.

import random
import string
import subprocess
from concurrent.futures import ThreadPoolExecutor


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


def run_client(_):
    subprocess.run(["./client", "127.0.0.1", random_string(20, 30)])


def main():
    with ThreadPoolExecutor(max_workers=50) as executor:
        executor.map(run_client, range(1, 51))


if __name__ == "__main__":
    main()
