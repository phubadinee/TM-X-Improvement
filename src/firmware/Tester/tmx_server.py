import socket
import time

HOST = '0.0.0.0'
PORT = 8600



def start_mock_tmx():
    print(f"[Mock TM-X] Starting server on Port {PORT}...")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen()
        print("[Mock TM-X] Waiting for connection from Pi...")
        i = 1
        while True:
            conn, addr = s.accept()
            with conn:
                print(f"\n[Mock TM-X] Connected by {addr}")
                latest_vals = "5.05,5.01" # ค่าสำรองเริ่มต้น
                try:
                    while True:
                        data = conn.recv(1024)
                        if not data:
                            break
                        
                        command = data.decode('ascii', errors='ignore').strip()
                        print(f"[Mock TM-X] Received command: {command}")
                        
                        if command.startswith("R0"):
                            conn.sendall(b"R0\r")
                        elif command.startswith("PW"):
                            conn.sendall(b"PW\r")
                            
                        # 1. เมื่อ Pi สั่ง T1 (Trigger) -> ให้เราพิมพ์ค่าที่ต้องการตรงนี้
                        elif command == 'T1':
                            # val_input = input("Enter measurement values (e.g. 5.05,5.01): ")
                
                            # time.sleep();
                            # ตอบรับ T1 กลับไปว่าสำเร็จ

                            
                            # if (input("Test [T1 , 03] : ") == "T1"):
                            #     conn.sendall(b"T1\r")
                            # else:
                            #     conn.sendall(b"T1,03")

                            if i==1:
                                conn.sendall(b"T1\r") 
                            elif i==2:
                                conn.sendall(b"T1,03") 
                                i-=1
                            elif i==3:
                                conn.sendall(b"T1\r")
                            
                            i+=1
                            print(f"[Mock TM-X] T1 acknowledged. Saved values: {latest_vals}")
                            
                        # 2. เมื่อ Pi สั่ง GM (Get Measurement) -> ดึงค่าที่เราเพิ่งพิมพ์ ส่งกลับไปให้ Pi
                        elif command.startswith("GM"):
                            try:
                                val_input = "6.0,5.0"
                                parts = val_input.split(',')
                                v1 = parts[0].strip()
                                v2 = parts[1].strip() if len(parts) > 1 else "5.01"
                            except:
                                v1, v2 = "5.05", "5.01"
                                
                            # จัดรูปแบบตามที่ฟังก์ชัน parse_gm ของ Pi ต้องการ (GM,จำนวน, X,สถานะ,ตัดสิน, Y,สถานะ,ตัดสิน)
                            response = f"GM,2,{v1},1,0,{v2},1,0\r"
                            conn.sendall(response.encode('ascii'))
                            print(f"[Mock TM-X] Sent GM data to Pi: {response.strip()}")
                            
                        else:
                            conn.sendall(b"0\r")
                            
                except Exception as e:
                    print(f"[Mock TM-X] Connection closed/error: {e}")

if __name__ == "__main__":
    start_mock_tmx()