import os
import socket

# [WO]
# THIS TEST THING WAS MADE BY AI
# I must confess, I was to lazy to attempt to write this I just wanted to test sending multiple messages
# w/ the raw byte version of the protocol

# Configure your server details here
HOST = "127.0.0.1"
PORT = 7080


def main():
  print(f"Connecting to server at {HOST}:{PORT}...")
  try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
  except ConnectionRefusedError:
    print("[-] Connection refused. Make sure your C server is running first.")
    return

  print("[+] Connected successfully!")
  print("Instructions:")
  print(" - Enter the path to your binary (.bin) file to send it.")
  print(" - Type 'exit', 'quit', or 'q' to stop.\n")

  try:
    while True:
      file_path = input("Binary file path > ").strip()

      # Check for exit sentinel
      if file_path.lower() in ["exit", "quit", "q"]:
        print("Exiting and closing connection...")
        break

      if not file_path:
        continue

      # Clean up path if dragged & dropped into terminal (removes outer quotes)
      file_path = file_path.strip('"\'')

      if not os.path.isfile(file_path):
        print(f"[-] Error: File '{file_path}' does not exist.")
        continue

      # Read file in binary mode and send
      try:
        with open(file_path, "rb") as f:
          binary_data = f.read()

        s.sendall(binary_data)
        print(
            f"[+] Successfully sent {len(binary_data)} bytes from"
            f" '{file_path}'."
        )

        # Optional: Uncomment below if your C server sends a response back
        # response = s.recv(1024)
        # if response:
        #     print(f"    Server response: {response.hex()}")

      except Exception as e:
        print(f"[-] Error reading or transmitting file: {e}")
        break

  finally:
    s.close()
    print("[-] Connection closed.")


if __name__ == "__main__":
  main()