import serial
import time
import glob
import sys

# ── Config ────────────────────────────────────────────────────────────────────
BAUD_RATE   = 115200
TIMEOUT_SEC = 1

# Protocol tokens
CMD_TRIGGER     = "[TRIGGER_TMX]"
ACK_TRIGGER     = "[TRIGGER_TMX_ACK]"
# ─────────────────────────────────────────────────────────────────────────────


def find_mega_port() -> str | None:
    """Return the first USB/ACM port where the Mega 2560 is detected."""
    ports = glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*")
    return ports[0] if ports else None


def receive(ser: serial.Serial) -> str | None:
    """
    Read one line from the Mega.
    Returns the decoded, stripped string, or None if nothing arrived.
    """
    if ser.in_waiting > 0:
        try:
            line = ser.readline().decode("utf-8").strip()
            if line:
                return line
        except UnicodeDecodeError:
            pass
    return None


def send(ser: serial.Serial, message: str) -> None:
    """Send a message string (+ newline) to the Mega."""
    ser.write((message + "\n").encode("utf-8"))
    print(f"[TX → Mega] {message}")


def handle_message(ser: serial.Serial, msg: str) -> None:
    """
    Dispatch logic for every message received from the Mega.
    Add more elif branches here as the protocol grows.
    """
    print(f"[RX ← Mega] {msg}")

    if msg == CMD_TRIGGER:
        # Mega is asking Pi to trigger TM-X measurement
        print("[INFO] Trigger received — sending ACK")
        send(ser, ACK_TRIGGER)

    # ── extend protocol here ──────────────────────────────────────────────
    # elif msg == "[OTHER_CMD]":
    #     send(ser, "[OTHER_CMD_ACK]")
    # ─────────────────────────────────────────────────────────────────────


def pi_uart() -> None:
    """
    Main UART loop.
    Connects to the Mega 2560 and continuously:
      - Receives commands
      - Dispatches to handle_message()
    """
    port = find_mega_port()
    if not port:
        print("[ERROR] No Arduino/Mega found on USB. Check connection.")
        sys.exit(1)

    print(f"[INFO] Connecting to Mega on {port} @ {BAUD_RATE} baud ...")

    try:
        with serial.Serial(port, BAUD_RATE, timeout=TIMEOUT_SEC) as ser:
            ser.reset_input_buffer()
            time.sleep(2)   # Let Mega finish boot / auto-reset
            print("[INFO] Ready. Waiting for commands from Mega...\n")

            while True:
                msg = receive(ser)
                if msg:
                    handle_message(ser, msg)

    except serial.SerialException as e:
        print(f"[ERROR] Serial: {e}")
        print("[HINT]  Run: sudo usermod -a -G dialout $USER  then reboot")
    except KeyboardInterrupt:
        print("\n[INFO] Stopped by user.")


if __name__ == "__main__":
    pi_uart()
