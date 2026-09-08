def handle_message(ser: serial.Serial, msg: str) -> None:
    # พิมพ์ Log ทุกข้อความที่ได้รับจาก Mega
    print(f"[RX ← Mega] {msg}")

    # --- โค้ดส่วน Test Communication ที่ต้องเพิ่มเข้าไป ---
    if msg == "<TEST_COMM>":
        print("[INFO] Received Comm Test Request from Mega. Sending ACK...")
        
        # ตอบกลับ Mega ทันที
        ack_msg = "<TEST_OK>\n"
        ser.write(ack_msg.encode("utf-8"))
        print(f"   [TX → Mega] {ack_msg.strip()}")
        return # จบการทำงานลูปนี้
    # -----------------------------------------------

    # --- โค้ดเดิมที่คุณมีอยู่แล้ว (เช่น ตรวจจับการ Trigger TM-X) ---
    elif msg == CMD_TRIGGER: # "<TRIGGER_TMX>"
        print("[INFO] Trigger received — Contacting TM-X via TCP...")
        # ... (โค้ด Trigger TM-X เดิม) ...