import socket

# รับการเชื่อมต่อจากทุก IP ในวง LAN
HOST = '0.0.0.0'
PORT = 8600 #[cite: 4]

def start_mock_tmx():
    print(f"[Mock TM-X] Starting server on Port {PORT}...")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        print("[Mock TM-X] Waiting for connection from Pi...")
        
        while True:
            conn, addr = s.accept()
            with conn:
                print(f"\n[Mock TM-X] Connected by {addr}")
                data = conn.recv(1024)
                
                if data:
                    command = data.decode('ascii').strip()
                    print(f"[Mock TM-X] Received command: {command}")
                    
                    if command == 'T1': #[cite: 4]
                        # สามารถเปลี่ยนเลขตรงนี้เพื่อทดสอบ Condition ต่างๆ ฝั่ง Pi 
                        # เช่น เปลี่ยนเป็น "5.02,5.01\r" เพื่อจำลองค่า OK
                        response = "5.05,5.01\r" 
                        conn.sendall(response.encode('ascii'))
                        print(f"[Mock TM-X] Sent data: {response.strip()}")

if __name__ == "__main__":
    start_mock_tmx()