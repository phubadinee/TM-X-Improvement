void runTrigWaitTMX() {
  showActionMessage("Measuring (TM-X)...");

  while (Serial.available() > 0) Serial.read();

  // Send to Rasp pi
  Serial.println("<TRIGGER_TMX>");

  bool receivedAck = false;
  while (!receivedAck) {
    if (Serial.available()) {
      String response = Serial.readStringUntil('\n');
      response.trim();

      if (response == "<MEASURE_OK>") {
        for_beep();
      } else {
        long_beep();
      }

      receivedAck = true;

    }
  }

  Serial.println("Measurement finished.");
}

void  runCommunicationTesting() {

  showActionMessage("Test Communication");
  // 1. เคลียร์ข้อมูลเก่าที่อาจค้างอยู่ใน Buffer ทิ้งก่อน
  while (Serial.available() > 0) Serial.read();

  // 2. แสดงข้อความบนจอ OLED ให้ User ทราบ
  // showActionMessage("Testing Pi Comm..."); 
  Serial.println("Testing Pi Comm...");

  // 3. ส่งคำสั่ง Ping ไปหา Raspberry Pi
  Serial.println("<TEST_COMM>");

  bool isConnected = false;
  unsigned long startTime = millis();
  unsigned long timeout = 2000; // ตั้งเวลา Timeout ไว้ที่ 2 วินาที ป้องกันระบบค้าง[cite: 7]

  // 4. วนลูปรอคำตอบกลับจนกว่าจะหมดเวลา
  while (millis() - startTime < timeout) {
    if (Serial.available()) {
      String response = Serial.readStringUntil('\n');
      response.trim();

      // ตรวจสอบว่า Pi ตอบกลับมาถูกต้องหรือไม่
      if (response == "<TEST_OK>") {
        isConnected = true;
        break; // ได้รับคำตอบแล้ว ให้ออกจากลูปทันที
      }
    }
  }

  // 5. สรุปผลและแสดงบนหน้าจอ OLED
  if (isConnected) {
    // showActionMessage("Comm: OK");
    // for_beep(); // เสียงบิ๊ปสั้นว่าปกติ
    Serial.println("Result: Communication OK");
  } else {
    // showActionMessage("Comm: FAILED!");
    // long_beep(); // เสียงบิ๊ปยาวเตือนว่าเชื่อมต่อไม่ได้
    Serial.println("Result: Communication FAILED (Timeout)");
  }
  
  delay(2000); // หน่วงเวลาให้ User อ่านผลลัพธ์บนจอ 2 วินาที ก่อนเด้งกลับไปหน้าเมนูหลัก
}