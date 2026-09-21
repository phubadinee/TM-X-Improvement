void runStart() {
  
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
