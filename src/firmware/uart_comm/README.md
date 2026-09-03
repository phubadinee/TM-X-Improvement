# USB Serial Communication between Raspberry Pi 5 and Arduino Mega 2560

This folder contains the code for two-way USB Serial communication between an Arduino Mega 2560 and a Raspberry Pi 5 using a standard USB cable.

## Hardware Setup

**No special wiring needed!**
Simply connect the Arduino Mega 2560 to the Raspberry Pi 5 using a standard **USB A to B cable** (the one normally used to program the Arduino). 
This completely bypasses the 3.3V vs 5V logic level problem because the USB handles it safely.

## 1. Arduino Mega Setup

1. Connect the Mega to your computer (not the Pi yet) to upload the code.
2. Open `mega/mega.ino` in the Arduino IDE.
3. Upload the code to the Mega.
4. **Unplug the Mega from your computer and plug it into the Raspberry Pi via USB.**

## 2. Raspberry Pi 5 Setup

1. **Install PySerial:**
   - Run `pip install pyserial` or `sudo apt install python3-serial`.
2. **Permissions (Important):**
   - The Pi user needs permission to access the USB serial ports. Run this in your terminal:
     `sudo usermod -a -G dialout $USER`
   - You may need to log out and log back in (or reboot the Pi) for this permission to take effect.
3. **Run the Script:**
   - Navigate to the `pi` folder.
   - Run the code: `python3 pi_uart.py`.
   - The script will automatically scan for connected USB devices (like `/dev/ttyACM0` or `/dev/ttyUSB0`) and connect to the Arduino.

## 3. Testing Communication

- **Mega -> Pi:** The Mega code is set up to automatically send a "Heartbeat" message to the Pi every 5 seconds. You will see these appear in the Pi's terminal without you typing anything.
- **Pi -> Mega:** Type a message in the Pi's terminal and press Enter. The Mega will receive it, process it, and send a response back confirming it received your exact message.
