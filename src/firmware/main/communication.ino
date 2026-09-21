char runTrigWaitTMX(int showlog) {

  if (showlog == 1) {
    showActionMessage("Measuring (TM-X)...");
  }
  
  char result = '0';

  // เคลียร์บัฟเฟอร์ แต่ถ้ามีคำสั่ง <STOP> ค้างอยู่ให้ยกเลิกเลย
  while (Serial.available() > 0) {
    String dump = Serial.readStringUntil('\n');
    dump.trim();
    if (dump == "<STOP>") return 'X';
  }

  // ส่งคำสั่งถ่ายภาพ
  Serial.println("<TRIGGER_TMX>");

  bool receivedAck = false;
  while (!receivedAck) {

    // --- ตรวจจับ Serial ---
    if (Serial.available()) {
      String response = Serial.readStringUntil('\n');
      response.trim();

      if (response == "<MEASURE_OK>") {
        for_beep();
        result = '1';
        receivedAck = true;
      }
      else if (response == "<MEASURE_NG>") {
        long_beep();
        result = '0';
        receivedAck = true;
      }
      // เพิ่มการดักจับ <STOP>
      else if (response == "<STOP>") {
        Serial.println("⚠️ Measurement Aborted via Serial!");
        return 'X'; // คืนค่า 'X' ทันที
      }
    }

    // --- ตรวจจับปุ่มกด ---
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        while (digitalRead(STOP_BTN_PIN) == LOW);
        return 'X';
      }
    }
  }

  Serial.println("Measurement finished.");
  return result;
}


char runTrigWaitTMX_TEST() {
  showActionMessage("Measuring (TM-X)...");

  int result;
  result = '0';

  Serial.println("Measurement finished.");
  return result;
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
     showActionMessage("Comm: OK");
     for_beep(); // เสียงบิ๊ปสั้นว่าปกติ
    Serial.println("Result: Communication OK");
  } else {
     showActionMessage("Comm: FAILED!");
     long_beep(); // เสียงบิ๊ปยาวเตือนว่าเชื่อมต่อไม่ได้
    Serial.println("Result: Communication FAILED (Timeout)");
  }

  delay(2000); // หน่วงเวลาให้ User อ่านผลลัพธ์บนจอ 2 วินาที ก่อนเด้งกลับไปหน้าเมนูหลัก
}



// ฟังก์ชันรอรับข้อมูล Package จาก Serial
// คืนค่ากลับมาเป็น 1, 2, หรือ 3 (ตามระยะที่จะให้ยืด) หากกด Stop จะคืนค่า -1
// ฟังก์ชันรอรับข้อมูล Package จาก Serial
int waitForPackageType() {
  //  showActionMessage("Waiting Pkg Data...");
  Serial.println("Waiting for <PKG:WxH> command...");

  while (true) {
    // 1. ตรวจสอบข้อมูลจาก Serial
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim(); // ตัดช่องว่างทิ้งรอบแรก

      // เช็คว่าขึ้นต้นด้วย <PKG: และลงท้ายด้วย > หรือไม่
      if (cmd.startsWith("<PKG:") && cmd.endsWith(">")) {

        // ตัดเอาเฉพาะข้อความข้างใน (เช่น <PKG:4x5> จะได้คำว่า 4x5)
        String pkgSize = cmd.substring(5, cmd.length() - 1);

        // ==========================================
        // [แก้ปัญหาที่นี่] ตัดช่องว่าง/อักขระซ่อนทิ้งอีกรอบ!
        pkgSize.trim();
        // ==========================================

        // โชว์ผลลัพธ์โดยครอบ [ ] ไว้ เพื่อเช็คว่ามีช่องว่างซ่อนอยู่หรือไม่
        Serial.print("Received Package Size: [");
        Serial.print(pkgSize);
        Serial.println("]");

        // เช็คในกลุ่มที่ 1
        for (int i = 0; i < sizeGroup1; i++) {
          if (pkgSize == pkgGroup1[i]) {
            lastReceivedPkg = pkgSize; // บันทึกชื่อจริงเก็บไว้
            Serial.println("Matched: Group 1 (Short)");
            return 1;
          }
        }

        // เช็คในกลุ่มที่ 2
        for (int i = 0; i < sizeGroup2; i++) {
          if (pkgSize == pkgGroup2[i]) {
            lastReceivedPkg = pkgSize; // บันทึกชื่อจริงเก็บไว้
            Serial.println("Matched: Group 2 (Mid)");
            return 2;
          }
        }

        // เช็คในกลุ่มที่ 3
        for (int i = 0; i < sizeGroup3; i++) {
          if (pkgSize == pkgGroup3[i]) {
            lastReceivedPkg = pkgSize; // บันทึกชื่อจริงเก็บไว้
            Serial.println("Matched: Group 3 (Long)");
            return 3;
          }
        }

        // ถ้าไม่ตรงกับลิสต์ไหนเลย ให้แจ้งเตือนและส่งค่า Default
        Serial.println("⚠️ Unknown Package Size! Defaulting to Type 1");
        return 1;
      }

      // ดักจับคำสั่ง <STOP>
      else if (cmd == "<STOP>") {
        Serial.println("⚠️ Aborted via Serial!");
        return -1;
      }
    }

    // 2. ดักจับการกดปุ่ม STOP/BACK
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        while (digitalRead(STOP_BTN_PIN) == LOW);
        Serial.println("⚠️ Aborted by User!");
        return -1;
      }
    }
  }
}
