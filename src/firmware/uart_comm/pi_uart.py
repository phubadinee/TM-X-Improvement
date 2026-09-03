import serial
import time
import threading
import sys
import glob

# Set the baud rate to match the Arduino code
BAUD_RATE = 115200

def find_arduino_port():
    """Automatically find the USB port the Arduino is connected to"""
    # Arduino Megas usually show up as ttyACM* or ttyUSB*
    ports = glob.glob('/dev/ttyACM*') + glob.glob('/dev/ttyUSB*')
    if not ports:
        return None
    return ports[0] # Return the first found port

def read_from_port(ser):
    """Background thread to continuously read data from USB Serial"""
    while True:
        try:
            if ser.in_waiting > 0:
                reading = ser.readline().decode('utf-8').strip()
                if reading:
                    # Print received message and reprint the input prompt
                    sys.stdout.write(f"\r\033[K[Received from Mega] {reading}\n")
                    sys.stdout.write("Enter message to send: ")
                    sys.stdout.flush()
        except Exception as e:
            print(f"\nError reading from serial: {e}")
            break

def main():
    SERIAL_PORT = find_arduino_port()
    
    if not SERIAL_PORT:
        print("Error: Could not find an Arduino connected via USB.")
        print("Please ensure the Arduino is plugged into the Raspberry Pi.")
        return

    try:
        # Open serial port
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        ser.flush()
        
        # When opening USB serial, the Arduino might auto-reset. 
        # Wait a moment for it to boot up.
        print(f"Connected to {SERIAL_PORT} at {BAUD_RATE} baud. Waiting 2 seconds for Arduino to boot...")
        time.sleep(2)
        
        # Start a thread to listen for incoming data
        read_thread = threading.Thread(target=read_from_port, args=(ser,))
        read_thread.daemon = True
        read_thread.start()
        
        # Main loop to send data
        print("\nType your message and press Enter to send to Mega.")
        print("Type 'exit' to quit.\n")
        
        while True:
            # We use sys.stdout for better formatting with the async receive thread
            sys.stdout.write("Enter message to send: ")
            sys.stdout.flush()
            message = sys.stdin.readline().strip()
            
            if message.lower() == 'exit':
                break
                
            if message:
                # Send the message with a newline character
                ser.write((message + '\n').encode('utf-8'))
                sys.stdout.write(f"\r\033[K[Sent to Mega] {message}\n")
            
    except serial.SerialException as e:
        print(f"\nSerial Error: {e}")
        print("\nTroubleshooting:")
        print("1. Check permissions: Run 'sudo usermod -a -G dialout $USER' and reboot your Pi.")
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == '__main__':
    main()
