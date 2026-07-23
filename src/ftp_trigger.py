import os
import glob
import threading
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer

# ================= CONFIGURATION =================
IMAGE_DIR = r"D:\\MatchaLatte\\TM-X Improvement\\images"
FTP_USER = "INTERN_USER"
FTP_PASS = "123456"
FTP_PORT = 21
FTP_IP = "0.0.0.0"
# =================================================

def start_ftp_server():
    """Runs the FTP server in the background."""
    # Ensure the directory exists so the script doesn't crash
    os.makedirs(IMAGE_DIR, exist_ok=True)
    
    authorizer = DummyAuthorizer()
    authorizer.add_user(FTP_USER, FTP_PASS, IMAGE_DIR, perm="elradfmw")
    
    handler = FTPHandler
    handler.authorizer = authorizer
    
    server = FTPServer((FTP_IP, FTP_PORT), handler)
    server.serve_forever()

def get_latest_image(directory):
    """Finds the most recently saved file in the directory."""
    # Get all files in the directory
    list_of_files = glob.glob(os.path.join(directory, '*'))
    if not list_of_files:
        return None
    
    # Return the file with the most recent modification time
    latest_file = max(list_of_files, key=os.path.getmtime)
    return latest_file

def main():
    # 1. Start the FTP server in a background (daemon) thread
    # daemon=True ensures the thread will close automatically when the main program stops
    ftp_thread = threading.Thread(target=start_ftp_server, daemon=True)
    ftp_thread.start()
    
    print(f"✅ FTP Server is running in the background on {FTP_IP}:{FTP_PORT}")
    print(f"📥 Images are being saved to: {IMAGE_DIR}")
    print("-" * 60)
    print("⌨️  Press [ENTER] to fetch the latest image.")
    print("🛑 Press [Ctrl + C] to stop the program.")
    print("-" * 60)

    # 2. Main loop for keyboard input
    try:
        while True:
            input()  # Pause and wait for the user to press Enter
            
            latest_image = get_latest_image(IMAGE_DIR)
            if latest_image:
                print(f"📸 Latest image captured: {latest_image}")
                
                # ---> ADD YOUR IMAGE PROCESSING LOGIC HERE <---
                # Example (if using OpenCV):
                # import cv2
                # img = cv2.imread(latest_image)
                # cv2.imshow("Keyence Image", img)
                # cv2.waitKey(1)
                
            else:
                print("⚠️ No images found in the directory yet. Waiting for Keyence to send...")
                
    except KeyboardInterrupt:
        # 3. Handle Ctrl+C gracefully without throwing massive traceback errors
        print("\n🛑 Ctrl+C detected. Shutting down the FTP Server and exiting safely...")

if __name__ == "__main__":
    main()