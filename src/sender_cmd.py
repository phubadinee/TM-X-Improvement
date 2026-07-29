import socket

# ================= TM-X CONFIGURATION =================
TMX_IP = "192.168.10.11" 
TMX_COMMAND_PORT = 8600       
TRIGGER_COMMAND = "T1\r"     
# ======================================================

def send_trigger():
    """Sends an ASCII command via TCP/IP to trigger the sensor."""
    try:
        # Establish a socket connection and send the command
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(2.0)  # Set a 2-second timeout for the connection
            s.connect((TMX_IP, TMX_COMMAND_PORT))
            s.sendall(TRIGGER_COMMAND.encode('ascii'))
            print("Trigger command sent successfully.")
            
    except socket.timeout:
        print("Error: Connection timed out. Please verify the IP address and network connection.")
    except ConnectionRefusedError:
        print(f"Error: Connection refused on port {TMX_COMMAND_PORT}. Please check sensor settings.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

def main():
    print(f"Command Sender initialized (Target: {TMX_IP}:{TMX_COMMAND_PORT})")
    print("-" * 60)
    print("Press [ENTER] to send the image capture trigger.")
    print("Press [Ctrl + C] to exit the program.")
    print("-" * 60)

    try:
        while True:
            input()  # Pause execution and wait for the user to press Enter
            send_trigger()
            
    except KeyboardInterrupt:
        print("\nStopping Command Sender safely...")

if __name__ == "__main__":
    main()