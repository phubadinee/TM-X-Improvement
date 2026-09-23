// ฟังก์ชันสำหรับรอให้ผู้ใช้หยิบชิ้นงานออกจากแท่น
int waitForPartRemoval() {
  Serial.println("Waiting for user to remove part...");
  delay(500); // หน่วงเวลาให้คนกำลังเอื้อมมือมาหยิบ

  int detect_val = read_st188();

  // ขณะที่ค่าน้อยกว่า 80 (แปลว่ายังมีของวางอยู่) ให้รอไปเรื่อยๆ
  while (detect_val < st188Threshold) {

    checkEmergencyReboot();
    
    // ดักจับคำสั่ง <STOP>
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      if (cmd == "<STOP>") {
        Serial.println("⚠️ Aborted while waiting for removal!");
        return -1;
      }
    }

    // ดักจับปุ่ม STOP
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        while (digitalRead(STOP_BTN_PIN) == LOW);
        return -1;
      }
    }

    detect_val = read_st188();
  }

  Serial.println("Part removed successfully.");
  return 1;
}


// ----------------------------------------------------
// อัปเดต runStart()
// ----------------------------------------------------
void runStart() {
  int itemCounter = 0;
  String currentPkgName = "---";
  String currentStatus = "Homing";

  printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);
  runHoming();
  for_beep_fast();

  for (int i = 0; i < 4; i++) {
    slotCounts[i] = 0;
  }

  while (1) {
    checkEmergencyReboot();
    
    for_beep_fast();

    currentStatus = "Wait PKG...";
    printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter + 1), "Status: " + currentStatus);

    int pkgTargetLevel = waitForPackageType();
    //    int pkgTargetLevel = waitForPackageType_TEST();
    if (pkgTargetLevel == -1) {
      long_beep();
      break;
    }

    itemCounter++;
    currentPkgName = lastReceivedPkg;

    if (Serial.available() > 0) {
      String command = Serial.readStringUntil('\n');
      command.trim();
      if (command == "<STOP>") {
        long_beep();
        break;
      }
    }

    // ==================================================
    // 3 & 4. ตรวจจับชิ้นงาน และ ยืดก้าน (มีระบบ Retry ในชิ้นเดิม)
    // ==================================================
    bool abortProcess = false;

    while (true) {
      
      checkEmergencyReboot();
      
      currentStatus = "Detecting...";
      printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);

      if (runDetectPart(0) == -1) {
        abortProcess = true;
        break;
      }

      currentStatus = "Aligning...";
      printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);

      if (extendPart_check(pkgTargetLevel) == 1) {
        break;
      }
      else {
        retractPart();
        long_beep();
        printBigResult2("ALIGN FAILED");
        Serial.println("<ERROR:ALIGN_STALL>");
        delay(1500);

        currentStatus = "Re-Place!";
        printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);

        if (waitForPartRemoval() == -1) {
          abortProcess = true;
          break;
        }
      }
    }

    if (abortProcess) {
      long_beep();
      break;
    }

    // 5. สถานะ: กำลังวัดผลจากกล้อง (Measuring TMX)
    currentStatus = "Measuring...";
    printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);

    char result_ = runTrigWaitTMX(0);
    //    char result_ = runTrigWaitTMX_TEST();
    if (result_ == 'X') {
      retractPart();
      long_beep();
      break;
    }

    // 🖥️ โชว์ผลลัพธ์ OK/NG ทันที แล้วบันทึกเวลาเริ่มต้นไว้
    String measureResult = (result_ == '1') ? "OK" : "NG";
    printBigResult(measureResult);
    unsigned long resultShowTime = millis();

    // ==================================================
    // ตรวจสอบเงื่อนไข Auto Sort (ทำงานทันที ไม่ต้องรอ delay)
    // ==================================================
    if (autoSortEnabled == true) {

      // ⚡ ให้ก้านหดกลับทันที (หน้าจอจะโชว์ OK/NG ค้างไว้ระหว่างมอเตอร์วิ่ง)
      retractPart();

      // ถดก้านเสร็จแล้ว แต่เวลายังผ่านไปไม่ถึง 1 วินาที (กลัวอ่านหน้าจอไม่ทัน) ให้รอจนครบ
      while (millis() - resultShowTime < 1000) {
        // ชดเชยเวลาให้คนอ่านจอ
      }

      currentStatus = "Sorting...";
      printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);
      runSortExecute(result_);

      currentStatus = "Pushing...";
      printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);
      runTransitionPush(0);

    } else {
      // โหมด Manual: ดึงก้านกลับทันทีเช่นกัน
      retractPart();

      // ชดเชยเวลาให้อ่านผลลัพธ์
      while (millis() - resultShowTime < 1000) { }

      currentStatus = "Pick up Part!";
      printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), " " + currentStatus);

      if (waitForPartRemoval() == -1) {
        long_beep();
        break;
      }
    }
    // ==================================================

    for_beep();
  }

  currentMenu = 0;
  cursorIndex = 0;
  scrollOffset = 0;
  updateDisplay();
}

void runStart2() {

  int itemCounter = 0;
  String currentPkgName = "---";  // เริ่มต้นยังไม่มีชื่อ PKG
  String currentStatus = "Homing";

  printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);
  runHoming();
  for_beep_fast();

  // รีเซ็ตค่าตัวนับชิ้นงานและจำนวนในราง
  for (int i = 0; i < 4; i++) {
    slotCounts[i] = 0;
  }

  while (1) {
    checkEmergencyReboot();
    for_beep_fast();

    // สถานะ: กำลังรอรับค่า <PKG>
    currentStatus = "Wait PKG...";
    printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter + 1), "Status: " + currentStatus);

    // 1. รอรับค่า <PKG:xxx>
    int pkgTargetLevel = waitForPackageType();

    if (pkgTargetLevel == -1) {
      Serial.println("⚠️ STOP Command Received during Wait. Exiting...");
      long_beep();
      break;
    }

    // เมื่อรับค่าได้แล้ว เพิ่มลำดับชิ้นงาน
    itemCounter++;

    // ใช้ชื่อแพ็กเกจจริงที่เพิ่งรับเข้ามาสดๆ ร้อนๆ
    currentPkgName = lastReceivedPkg; // เช่น "5x5", "8x8", "9x9"

    // 2. เช็คคำสั่ง <STOP>
    if (Serial.available() > 0) {
      String command = Serial.readStringUntil('\n');
      command.trim();
      if (command == "<STOP>") {
        Serial.println("⚠️ STOP Command Received. Exiting...");
        long_beep();
        break;
      }
    }

    // 3. สถานะ: กำลังตรวจจับชิ้นงาน (Detect Part)
    currentStatus = "Detecting...";
    printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);

    if (runDetectPart(0) == -1) {
      long_beep();
      break;
    }

    // 4. สถานะ: กำลังยืดก้าน (Extending)
    currentStatus = "Aligning...";
    printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);

    extendPart(pkgTargetLevel);

    // 5. สถานะ: กำลังวัดผลจากกล้อง (Measuring TMX)
    currentStatus = "Measuring...";
    printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);

    char result_ = runTrigWaitTMX(0);

    if (result_ == 'X') {
      Serial.println("Process Canceled during TM-X.");
      retractPart();
      long_beep();
      break;
    }

    // 🖥️ [หน้าจอพิเศษ]: เมื่อวัดเสร็จ แสดงผล OK หรือ NG ตัวใหญ่ๆ กลางจอ
    String measureResult = (result_ == '1') ? "OK" : "NG";
    printBigResult(measureResult);
    delay(1500); // หน่วงโชว์ตัวใหญ่ๆ 1.5 วินาที

    // 6. สถานะ: กำลังหดก้านและคัดแยก (Retracting & Sorting)
    currentStatus = "Sorting...";
    printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);

    retractPart();
    runSortExecute(result_);

    // 7. สถานะ: กำลังผลักชิ้นงานลงกล่อง (Pushing)
    currentStatus = "Pushing...";
    printStatus("PKG: " + currentPkgName, "Item No: " + String(itemCounter), "Status: " + currentStatus);

    runTransitionPush(0);

    for_beep();
  }

  // เมื่อหลุดออกจากลูป ให้รีเฟรชหน้าจอกลับไปเมนูหลัก
  currentMenu = 0;
  cursorIndex = 0;
  scrollOffset = 0;
  updateDisplay();
}

void runSystemHoming() {
  showActionMessage("System Homing...");

  retractPart();

  runTransitionRetract(0);
  runTransitionPush(0);

  runHoming();
  for_beep();
}


//void runEmergencyHalt() {
//  // 1. สั่งตัดไฟมอเตอร์ทันทีเพื่อความปลอดภัย
//  digitalWrite(enPin, HIGH);
//
//  // 2. สั่งดึงไฟขา 12 ลง GND ซึ่งจะไปดึงขา RESET ของบอร์ดให้ทำงาน
//  // บอร์ดจะดับและเปิดใหม่ทันที 100% เหมือนเอานิ้วกดปุ่มรีเซ็ต
//  digitalWrite(RESET_PIN, LOW);
//}
