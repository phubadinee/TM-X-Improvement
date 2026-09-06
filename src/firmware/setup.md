# Setting IP Addr

## Laptop (TM-X Mockup)
sudo ip addr add 192.168.0.11/24 dev eno1
sudo ip link set eno1 up

## Raspberrypi
sudo ip addr add 192.168.0.10/24 dev eth0
sudo ip link set eth0 up



# Firewall
sudo ufw status

if inactive -> pass (do nothing)
if active -> open Port8600
sudo ufw allow 8600/tcp

# Port Test
server (destination)
nc -l 8600

origin 
nc 192.168.0.11 8600


python mock_tmx.py
```[cite: 1, 13]

**ขั้นตอนที่ 2: รันฝั่ง Raspberry Pi 5 (`Pi_test.py`)**
เปิด Terminal บน Pi แล้วรันสคริปต์หลัก (ซึ่งตอนนี้มีโค้ด Serial เชื่อมกับ Mega ฝังอยู่แล้ว):
```bash
python Pi_test.py
```[cite: 1]
*ระบบจะทำการค้นหาพอร์ต USB ของ Mega อัตโนมัติ (เช่น `/dev/ttyACM0`) และพิมพ์ข้อความว่า `Mega Serial Connected Successfully` ใน Log*[cite: 1]

**ขั้นตอนที่ 3: สั่งเริ่มต้นการวัดผ่าน Web API (จำลอง Web App)**
เปิด Browser บนเครื่องไหนก็ได้ในวง LAN หรือใช้คำสั่ง cURL เพื่อส่งคำสั่งเปิด Session (เช่น สั่งวัด 3 ชิ้น)[cite: 1]:
*   เปิดลิงก์ในเบราว์เซอร์: `http://<IP_ของ_Pi>:9998/start?count=3`
*   เมื่อรันแล้ว Log บน Pi จะขึ้นว่า `⏳ รอสัญญาณจาก MCU ...` (ถูกต้องแล้ว ระบบกำลังจดจ่อรอสัญญาณจาก Mega)[cite: 1]

**ขั้นตอนที่ 4: สั่งงานจาก Arduino Mega (หน้างานจริง)**
*   ที่หน้าจอ OLED ของ Mega ให้ใช้ปุ่มหรือ Rotary Encoder เลือกเมนู **Trig & Wait TM-X** แล้วกดเลือก[cite: 7, 11]
*   Mega จะส่งสัญญาณ `<TRIGGER_TMX>` ผ่านสาย USB มาให้ Pi[cite: 11]
*   Pi จะวิ่งไปสั่ง `T1` ผ่าน TCP ไปหา Laptop (TM-X) $\rightarrow$ รับค่ากลับมาคำนวณเงื่อนไข $\rightarrow$ ส่งผลลัพธ์ (`<MEASURE_OK>` หรือ `<MEASURE_NG>`) กลับมาหา Mega[cite: 1, 11]
*   Mega ได้รับผลแล้วจะส่งเสียง Beep และทำกลไกคัดแยกชิ้นงานต่อไป[cite: 11]

**ขั้นตอนที่ 5: ตรวจสอบสถานะและข้อมูลการวัด**
คุณสามารถเช็กสถานะหรือข้อมูลจำลองที่บันทึกไว้ได้ตลอดเวลาผ่าน Browser[cite: 1]:
*   ดูสถานะระบบ: `http://<IP_ของ_Pi>:9998/status`[cite: 1]
*   ดูข้อมูลการวัดทั้งหมด: `http://<IP_ของ_Pi>:9998/mock/db`[cite: 1]