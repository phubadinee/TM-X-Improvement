// ฟังก์ชันสำหรับรอให้ผู้ใช้หยิบชิ้นงานออกจากแท่น
int waitForPartRemoval() {
  Serial.println("Waiting for user to remove part...");
  delay(500);

  int detect_val = read_st188();

  while (detect_val < st188Threshold) {
    checkEmergencyReboot();
    
    // ==================================================
    // จุดสำคัญ: ดักจับทั้ง STOP และ PKG ห้ามอ่านแล้วทิ้งเปล่าๆ
    // ==================================================
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      
      if (cmd == "<STOP>") {
        Serial.println("⚠️ Aborted while waiting for removal!");
        return -1;
      }
      else if (cmd.startsWith("<PKG:")) {
        pendingPKG = cmd; // ถ้า Pi ส่งงานชิ้นต่อไปมาตอนที่คนยังไม่หยิบของ ให้เก็บใส่กระเป๋าไว้!
      }
    }

    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        while (digitalRead(STOP_BTN_PIN) == LOW);
        return -1;
      }
    }

    detect_val = read_st188();
    delay(5); // พัก MCU
  }

  Serial.println("Part removed successfully.");
  return 1;
}
void runStart() {
  int itemCounter = 0;
  String currentPkgName = "---";
  lastReceivedItemID = "---"; // รีเซ็ต ID ตอนเริ่มเครื่อง
  String currentStatus = "Homing";

  printStatus("PKG: " + currentPkgName, "Item No: --- (0)", "Status: " + currentStatus);
  runHoming();
  for_beep_fast();

  for (int i = 0; i < 4; i++) {
    slotCounts[i] = 0;
  }

  while (1) {
    checkEmergencyReboot();
    for_beep_fast();

    currentStatus = "Wait PKG...";
    String waitItemStr = "--- (" + String(itemCounter + 1) + ")";
    printStatus("PKG: " + currentPkgName, "Item No: " + waitItemStr, "Status: " + currentStatus);

    int pkgTargetLevel = waitForPackageType();
    if (pkgTargetLevel == -1) {
      long_beep();
      break;
    }

    itemCounter++;
    currentPkgName = lastReceivedPkg;
    
    String displayItemStr = lastReceivedItemID + " (" + String(itemCounter) + ")";

    bool processComplete = false;
    bool abortProcess = false;
    char result_ = 'X';

    // ==================================================
    // ลูปรวม: ตรวจจับ -> จัดแนว -> วัดผล
    // ==================================================
    while (!processComplete) {

      // --- ลูปย่อย: จัดแนว (Align Retry) ---
      while (true) {
        checkEmergencyReboot();
        
        currentStatus = "Detecting...";
        printStatus("PKG: " + currentPkgName, "Item No: " + displayItemStr, "Status: " + currentStatus);

        for_beep_fast();
        if (runDetectPart(0) == -1) {
          abortProcess = true;
          break;
        }

        currentStatus = "Aligning...";
        printStatus("PKG: " + currentPkgName, "Item No: " + displayItemStr, "Status: " + currentStatus);

        if (extendPart_check(pkgTargetLevel) == 1) {
          break; // จัดแนวสำเร็จ ไปวัดผล
        }
        else {
          retractPart();
          long_beep();
          printBigResult2("ALIGN FAILED");
          Serial.println("<ERROR:ALIGN_STALL>");
          delay(1500);

          currentStatus = "Re-Place!";
          printStatus("PKG: " + currentPkgName, "Item No: " + displayItemStr, "Status: " + currentStatus);

          if (waitForPartRemoval() == -1) {
            abortProcess = true;
            break;
          }
        }
      }

      if (abortProcess) break;

      // --- วัดผล (Measuring) ---
      currentStatus = "Measuring...";
      printStatus("PKG: " + currentPkgName, "Item No: " + displayItemStr, "Status: " + currentStatus);

      result_ = runTrigWaitTMX(0);

      if (result_ == 'E') {
        Serial.println("⚠️ Measure Error! Retracting and Retrying...");
        retractPart();
        long_beep();
        printBigResult2("MEASURE ERROR");
        delay(1500);

        currentStatus = "Re-Place!";
        printStatus("PKG: " + currentPkgName, "Item No: " + displayItemStr, "Status: " + currentStatus);

        if (waitForPartRemoval() == -1) {
          abortProcess = true;
          break;
        }
        continue; 
      } 
      else if (result_ == 'X') {
        retractPart();
        long_beep();
        abortProcess = true;
        break;
      }

      processComplete = true; 
    }

    if (abortProcess) {
      long_beep();
      break; 
    }

    // ==================================================
    String measureResult = (result_ == '1') ? "OK" : "NG";
    printBigResult(measureResult);
    unsigned long resultShowTime = millis();

    if (autoSortEnabled == true) {
      retractPart();

      // ==================================================
      // [แก้ไข] ดักจับ PKG ระหว่างรอโชว์จอ 1 วินาที (Auto ON)
      // ==================================================
      while (millis() - resultShowTime < 1000) { 
        checkEmergencyReboot();
        if (Serial.available() > 0) {
          String cmd = Serial.readStringUntil('\n');
          cmd.trim();
          if (cmd.startsWith("<PKG:")) {
            pendingPKG = cmd; 
          }
        }
        delay(5);
      }
      // ==================================================

      currentStatus = "Sorting...";
      printStatus("PKG: " + currentPkgName, "Item No: " + displayItemStr, "Status: " + currentStatus);
      runSortExecute(result_);

      currentStatus = "Pushing...";
      printStatus("PKG: " + currentPkgName, "Item No: " + displayItemStr, "Status: " + currentStatus);
      runTransitionPush(0);
    } else {
      retractPart();

      // ==================================================
      // [แก้ไข] ดักจับ PKG ระหว่างรอโชว์จอ 1 วินาที (Auto OFF)
      // ==================================================
      while (millis() - resultShowTime < 1000) { 
        checkEmergencyReboot();
        if (Serial.available() > 0) {
          String cmd = Serial.readStringUntil('\n');
          cmd.trim();
          if (cmd.startsWith("<PKG:")) {
            pendingPKG = cmd; 
          }
        }
        delay(5);
      }
      // ==================================================

      currentStatus = "Pick up Part!";
      printStatus("PKG: " + currentPkgName, "Item No: " + displayItemStr, " " + currentStatus);

      if (waitForPartRemoval() == -1) {
        long_beep();
        break;
      }
    }

    Serial.println("======= Cycle Completed =======");
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
