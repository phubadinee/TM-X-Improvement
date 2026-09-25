char runTrigWaitTMX(int showlog) {

  if (showlog == 1) {
    showActionMessage("Measuring (TM-X)...");
  }
  
  char result = '0';

  // ==========================================
  // 1. เคลียร์บัฟเฟอร์: แต่ถ้าเจอคำสั่ง <PKG:...> ให้เก็บไว้ ห้ามทิ้งเด็ดขาด!
  // ==========================================
  while (Serial.available() > 0) {
    String dump = Serial.readStringUntil('\n');
    dump.trim();
    if (dump == "<STOP>") {
      return 'X';
    }
    else if (dump.startsWith("<PKG:")) {
      pendingPKG = dump; // เก็บใส่ตัวแปรพักข้อมูลไว้
    }
  }

  // ส่งคำสั่งถ่ายภาพให้คอมพิวเตอร์/Raspberry Pi
  Serial.println("<TRIGGER_TMX>");

  bool receivedAck = false;
  while (!receivedAck) {

    checkEmergencyReboot();
    
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
      // ดักจับ <MEASURE_ERROR> จาก Pi
      else if (response == "<MEASURE_ERROR>") {
        Serial.println("⚠️ Measurement Error via Serial! (Requesting Retry)");
        return 'E'; // ส่ง 'E' กลับไปให้ runStart ดึงก้านกลับแล้วเริ่มใหม่
      }
      // ดักจับ <STOP>
      else if (response == "<STOP>") {
        Serial.println("⚠️ Measurement Aborted via Serial!");
        return 'X'; // คืนค่า 'X' ทันที
      }
      // ==========================================
      // 2. ถ้า Pi ส่ง PKG มาระหว่างที่กล้องกำลังวัดผล ให้เก็บไว้เช่นกัน!
      // ==========================================
      else if (response.startsWith("<PKG:")) {
        pendingPKG = response;
      }
    }

    // --- ตรวจจับปุ่มกด STOP/BACK ---
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        while (digitalRead(STOP_BTN_PIN) == LOW);
        return 'X';
      }
    }
    
    delay(5); // ให้ MCU ได้พักจังหวะ ป้องกัน Serial Buffer ทำงานหนักเกินไป
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
    
    checkEmergencyReboot();
    
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


int waitForPackageType() {
  Serial.println("Waiting for <PKG:WxH:ID> command...");

  while (true) {
    checkEmergencyReboot();
    String cmd = "";

    // โค้ดดึงข้อมูล (รองรับ pendingPKG จากวิธีครั้งที่แล้ว)
    if (pendingPKG != "") {
      cmd = pendingPKG;
      pendingPKG = ""; 
    } 
    else if (Serial.available() > 0) {
      cmd = Serial.readStringUntil('\n');
      cmd.trim();
    }

    if (cmd != "") {
      if (cmd.startsWith("<PKG:") && cmd.endsWith(">")) {
        // ตัด <PKG: และ > ทิ้ง จะเหลือแค่ "9x9:547" หรือ "9x9"
        String data = cmd.substring(5, cmd.length() - 1);
        data.trim();

        String pkgSize = "";
        String itemID = "";
        int colonIndex = data.indexOf(':');

        // แยกขนาด กับ ID ออกจากกัน (ถ้ารูปแบบเป็น 9x9:547)
        if (colonIndex != -1) {
            pkgSize = data.substring(0, colonIndex);
            itemID = data.substring(colonIndex + 1);
        } else {
            // เผื่อไว้รองรับคำสั่งแบบเก่าที่ไม่มี ID (<PKG:9x9>)
            pkgSize = data; 
            itemID = "---";
        }
        
        pkgSize.trim();
        itemID.trim();
        
        lastReceivedItemID = itemID; // บันทึก ID ไว้ใช้งาน

        Serial.print("Received Package Size: [");
        Serial.print(pkgSize);
        Serial.print("], Item ID: [");
        Serial.print(itemID);
        Serial.println("]");

        for (int i = 0; i < sizeGroup1; i++) {
          if (pkgSize == pkgGroup1[i]) { lastReceivedPkg = pkgSize; return 1; }
        }
        for (int i = 0; i < sizeGroup2; i++) {
          if (pkgSize == pkgGroup2[i]) { lastReceivedPkg = pkgSize; return 2; }
        }
        for (int i = 0; i < sizeGroup3; i++) {
          if (pkgSize == pkgGroup3[i]) { lastReceivedPkg = pkgSize; return 3; }
        }
        
        Serial.println("⚠️ Unknown Package Size! Defaulting to Type 1");
        return 1;
      }
      else if (cmd == "<STOP>") {
        Serial.println("⚠️ Aborted via Serial!");
        return -1;
      }
    }

    // ดักจับปุ่มกด
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        while (digitalRead(STOP_BTN_PIN) == LOW);
        Serial.println("⚠️ Aborted by User!");
        return -1;
      }
    }
    
    delay(5); // ให้ MCU ได้พักจังหวะ
  }
}

int waitForPackageType_TEST() {
  Serial.println("Waiting for <PKG:WxH> command... (TEST MODE)");
  
  // หน่วงเวลา 0.5 วินาที ให้ผู้ใช้งานมองเห็นหน้าจอ "Wait PKG..." ได้ทัน
  delay(500); 

  // สมมติชื่อแพ็กเกจเพื่อนำไปโชว์บนหน้าจอ OLED
  String mockPkg = "TEST"; 
  lastReceivedPkg = mockPkg; 

  Serial.print("Received Package Size: [");
  Serial.print(mockPkg);
  Serial.println("] (Mocked)");
  
  Serial.println("Matched: TEST Group (Defaulting to Type 1)");

  // คืนค่าระดับการยืดก้านที่ 1 (Short) เพื่อให้ระบบทำงานในสเต็ปถัดไปต่อได้ทันที
  // (หากต้องการทดสอบระยะอื่น สามารถเปลี่ยนเป็น return 2 หรือ 3 ได้ครับ)
  return 1; 
}
