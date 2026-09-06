import serial
import time
import glob
import sys
import socket

# ── Config ────────────────────────────────────────────────────────────────────
BAUD_RATE   = 115200
TIMEOUT_SEC = 1

# Protocol tokens
CMD_TRIGGER     = "<TRIGGER_TMX>"
ACK_OK          = "<MEASURE_OK>"
ACK_NG          = "<MEASURE_NG>"

# TM-X TCP Config
TMX_IP          = "192.168.0.11" # เปลี่ยนเป็น IP ของ Laptop (Mock TM-X)
TMX_PORT        = 8600 #[cite: 4]
TMX_TIMEOUT     = 3.0
# ─────────────────────────────────────────────────────────────────────────────

def find_mega_port() -> str | None:
    """Return the first USB/ACM port where the Mega 2560 is detected."""
    ports = glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*")
    return ports[0] if ports else None

def receive(ser: serial.Serial) -> str | None:
    if ser.in_waiting > 0:
        try:
            line = ser.readline().decode("utf-8").strip()
            if line:
                return line
        except UnicodeDecodeError:
            pass
    return None

def send(ser: serial.Serial, message: str) -> None:
    ser.write((message + "\n").encode("utf-8"))
    print(f"[TX → Mega] {message}")

def trigger_tmx_tcp() -> str | None:
    """Send T1 trigger via TCP and return the raw string response."""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(TMX_TIMEOUT)
            s.connect((TMX_IP, TMX_PORT))
            
            # ส่งคำสั่ง T1 พร้อม Carriage Return (\r)[cite: 1, 4]
            s.sendall(b"T1\r") 
            
            # รับข้อมูลและตัดช่องว่าง/ขึ้นบรรทัดใหม่
            data = s.recv(1024).decode("ascii").strip()
            return data
    except Exception as e:
        print(f"[ERROR] TM-X Communication failed: {e}")
        return None

def handle_message(ser: serial.Serial, msg: str) -> None:
    print(f"[RX ← Mega] {msg}")

    if msg == CMD_TRIGGER:
        print("[INFO] Trigger received — Contacting TM-X via TCP...")
        
        tmx_data = trigger_tmx_tcp()
        
        if tmx_data:
            print(f"[INFO] TM-X Raw Data: {tmx_data}")
            try:
                # แยกค่าด้วยคอมม่าและแปลงเป็น Float
                val1_str, val2_str = tmx_data.split(',')
                val1 = float(val1_str)
                val2 = float(val2_str)
                
                # เช็คเงื่อนไข: มากกว่า 5.03 ทั้ง 2 ค่า ถือเป็น NG
                if val1 > 5.03 or val2 > 5.03:
                    print("[INFO] Condition: Both > 5.03 -> NG")
                    send(ser, ACK_NG)
                else:
                    print("[INFO] Condition: OK")
                    send(ser, ACK_OK)
                    
            except ValueError:
                print("[ERROR] Cannot parse TM-X data. Sent NG.")
                send(ser, ACK_NG)
        else:
            print("[ERROR] No response from TM-X. Sent NG.")
            send(ser, ACK_NG)


def pi_uart() -> None:
    port = find_mega_port()
    if not port:
        print("[ERROR] No Arduino/Mega found on USB. Check connection.")
        sys.exit(1)

    print(f"[INFO] Connecting to Mega on {port} @ {BAUD_RATE} baud ...")

    try:
        with serial.Serial(port, BAUD_RATE, timeout=TIMEOUT_SEC) as ser:
            ser.reset_input_buffer()
            time.sleep(2)
            print("[INFO] Ready. Waiting for commands from Mega...\n")

            while True:
                msg = receive(ser)
                if msg:
                    handle_message(ser, msg)

    except serial.SerialException as e:
        print(f"[ERROR] Serial: {e}")
    except KeyboardInterrupt:
        print("\n[INFO] Stopped by user.")

if __name__ == "__main__":
    pi_uart()