import os
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer

# ================= RECEIVER CONFIGURATION =================
IMAGE_DIR = r"D:\\MatchaLatte\\TM-X_Improvement\\images"
FTP_USER = "INTERN_USER"
FTP_PASS = "123456"
FTP_PORT = 21
FTP_IP = "0.0.0.0"
# =================================================

class CustomFTPHandler(FTPHandler):
    """Custom handler to trigger an action immediately upon receiving a file."""
    
    def on_file_received(self, file):
        print(f"\nNew image file received: {file}")

def run_ftp_server():
    # Ensure the destination directory exists to prevent errors
    os.makedirs(IMAGE_DIR, exist_ok=True)
    
    # Configure user credentials and directory permissions
    authorizer = DummyAuthorizer()
    authorizer.add_user(FTP_USER, FTP_PASS, IMAGE_DIR, perm="elradfmw")
    
    # Assign the custom handler to process incoming files
    handler = CustomFTPHandler
    handler.authorizer = authorizer
    
    # Initialize and start the FTP Server
    server = FTPServer((FTP_IP, FTP_PORT), handler)
    
    print(f"FTP Server is running at {FTP_IP}:{FTP_PORT}")
    print(f"Waiting for Keyence sensor images. Saving to: {IMAGE_DIR}")
    print("Press [Ctrl + C] to stop the server.")
    print("-" * 60)
    
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping FTP Server safely...")

if __name__ == "__main__":
    run_ftp_server()