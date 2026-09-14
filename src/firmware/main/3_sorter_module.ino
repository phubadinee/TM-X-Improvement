void runSortExecute(char result) {
  showActionMessage("Sort Execute...");
  Serial.println("======= [Start] Sort Execute =======");
  processSlotCommand(result);
  Serial.println("======= [End] Sort Execute ======="); Serial.println();

}

void processSlotCommand(char command) {
  int targetIndex = -1;


  if (command != 't') {
    // ถ้าได้รับ '1' (เลือกช่อง 1 หรือ 2)
    if (command == '1') {
      if (slotCounts[0] < 4) {
        targetIndex = 0; // ช่องที่ 1
        slotCounts[0]++;
      } else if (slotCounts[1] < 4) {
        targetIndex = 1; // ช่องที่ 2
        slotCounts[1]++;
      } else {
        Serial.println("⚠️ Warning: Slot 1 and 2 are FULL!");
      }
    }
    // ถ้าได้รับ '0' (เลือกช่อง 3 หรือ 4)
    else if (command == '0') {
      if (slotCounts[2] < 4) {
        targetIndex = 2; // ช่องที่ 3
        slotCounts[2]++;
      } else if (slotCounts[3] < 4) {
        targetIndex = 3; // ช่องที่ 4
        slotCounts[3]++;
      } else {
        Serial.println("⚠️ Warning: Slot 3 and 4 are FULL!");
      }
    }

    // ถ้าเลือกตำแหน่งได้ ให้สั่งมอเตอร์วิ่งไป
    if (targetIndex != -1) {
      Serial.print("Signal '");
      Serial.print(command);
      Serial.print("' -> Moving to Slot: ");
      Serial.print(targetIndex + 1);
      Serial.print(" (Items in this slot: ");
      Serial.print(slotCounts[targetIndex]);
      Serial.println("/4)");

      digitalWrite(enPin, LOW); // จ่ายไฟเตรียมวิ่ง
      stepper.runToNewPosition(positions[targetIndex]);

      // ถึงตำแหน่งแล้ว ตัดไฟพัก 1 วินาที
      pauseMotor();
    }
  } else {

    digitalWrite(enPin, LOW); // จ่ายไฟเตรียมวิ่ง
    stepper.runToNewPosition(positions[0]);
    for_beep_fast();
    stepper.runToNewPosition(positions[1]);
    for_beep_fast();
    stepper.runToNewPosition(positions[2]);
    for_beep_fast();
    stepper.runToNewPosition(positions[3]);
    // ถึงตำแหน่งแล้ว ตัดไฟพัก 1 วินาที

    runHoming();
    long_beep();
    pauseMotor();
  }
}


void runHoming() {
  Serial.println("Start Homing...");

  // ให้แน่ใจว่าเปิดไฟเลี้ยงมอเตอร์ก่อน Homing
  digitalWrite(enPin, LOW);

  // 1. ตั้งความเร็วให้วิ่งไปหาลิมิตสวิตช์
  stepper.setSpeed(-400);

  // 2. วนลูปสั่งมอเตอร์หมุน จนกว่าจะชนสวิตช์ (สถานะเป็น LOW)
  while (digitalRead(limit_sorter) == HIGH) {
    stepper.runSpeed();
  }

  // 3. เมื่อชนสวิตช์ (หลุดลูป) ให้ "บังคับหยุดแบบเด็ดขาด"
  stepper.setSpeed(0);             // รีเซ็ตความเร็ว
  stepper.setCurrentPosition(0);   // รีเซ็ตตำแหน่งปัจจุบันเป็น 0
  stepper.moveTo(0);               // เคลียร์เป้าหมายการวิ่ง (Target) ให้เป็น 0 ด้วย (สำคัญมาก!)

  Serial.println("Homing Complete! Motor is fully stopped.");

  // 4. ตัดไฟมอเตอร์เพื่อป้องกันมอเตอร์ล็อคค้างและร้อน (แกนจะหมุนอิสระได้)
  pauseMotor();
}


// ฟังก์ชันสำหรับหน่วงเวลาและตัดไฟมอเตอร์
void pauseMotor() {
  delay(1000);            // รอให้มอเตอร์หยุดนิ่งสนิท 1 วินาที
  digitalWrite(enPin, HIGH); // สั่งตัดไฟ (Disable) ค้างไว้ เพื่อลดการกินกระแส
  // ไม่ต้องสั่งเปิดไฟกลับตรงนี้ เพราะตอนจะวิ่งฟังก์ชันหลักจะเปิดให้อัตโนมัติ
}

void setSorterOffset() {
  int selectedSlot = 0;       // 0-3 คือ Slot 1-4, ส่วน 4 คือ Save & Exit
  bool adjustingMode = false;
  long tempPos = 0;
  int scrollOffsetAdj = 0;    // สำหรับเลื่อนหน้าจอเหมือนใน drawMenu
  const int maxVisibleAdj = 4;

  int lastClk = digitalRead(CLK_PIN);
  bool redraw = true;

  // --- วาดหน้าจอตอนกำลัง Homing ตามสไตล์ showActionMessage ---
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setDrawColor(1);
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.drawBox(0, 0, 128, 18);
  u8g2.setDrawColor(0);
  u8g2.setCursor(13, 13);
  u8g2.print(">>> EXECUTING <<<");
  u8g2.setDrawColor(1);
  u8g2.setCursor(10, 40);
  u8g2.print("Homing Sorter...");
  u8g2.sendBuffer();

  runHoming();
  digitalWrite(enPin, LOW); // เปิดไฟเลี้ยงมอเตอร์เตรียมขยับ

  while (true) {
    // 1. จัดการการวาดหน้าจอ OLED
    if (redraw) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tf);

      if (!adjustingMode) {
        // ==========================================
        // โหมดเลือกช่อง (สไตล์เดียวกับ drawMenu)
        // ==========================================
        u8g2.setDrawColor(1);
        u8g2.setCursor(2, 9);
        u8g2.print("ADJUST SORTER POS.");
        u8g2.drawLine(0, 12, 128, 12);

        // คำนวณการเลื่อนหน้าจอ
        if (selectedSlot >= scrollOffsetAdj + maxVisibleAdj) {
          scrollOffsetAdj = selectedSlot - maxVisibleAdj + 1;
        } else if (selectedSlot < scrollOffsetAdj) {
          scrollOffsetAdj = selectedSlot;
        }

        // วาดรายการเมนู
        for (int i = 0; i < maxVisibleAdj; i++) {
          int itemIndex = scrollOffsetAdj + i;
          if (itemIndex > 4) break;

          int yPos = 24 + (i * 12);

          if (itemIndex == selectedSlot) {
            u8g2.setDrawColor(1);
            u8g2.drawBox(0, yPos - 9, 128, 11);
            u8g2.setDrawColor(0); // ตัวหนังสือสีดำบนแถบขาว
          } else {
            u8g2.setDrawColor(1); // ตัวหนังสือปกติสีขาว
          }

          u8g2.setCursor(4, yPos);
          if (itemIndex < 4) {
            u8g2.print("Slot ");
            u8g2.print(itemIndex + 1);
            u8g2.print(": ");
            u8g2.print(positions[itemIndex]);
          } else {
            u8g2.print("Save & Exit");
          }
        }
      } else {
        // ==========================================
        // โหมดกำลังหมุนปรับค่า (สไตล์เดียวกับ showActionMessage)
        // ==========================================
        u8g2.setDrawColor(1);
        u8g2.drawFrame(0, 0, 128, 64);
        u8g2.drawBox(0, 0, 128, 18);

        u8g2.setDrawColor(0);
        u8g2.setCursor(13, 13);
        u8g2.print(">>> ADJUSTING <<<");

        u8g2.setDrawColor(1);
        u8g2.setCursor(10, 35);
        u8g2.print("Target: Slot ");
        u8g2.print(selectedSlot + 1);

        u8g2.setCursor(10, 50);
        u8g2.print("Pos: ");
        u8g2.print(tempPos);
      }
      u8g2.sendBuffer();
      redraw = false;
    }

    // 2. อ่านค่า Rotary Encoder
    int clkState = digitalRead(CLK_PIN);
    if (clkState != lastClk && clkState == HIGH) {
      int dtState = digitalRead(DT_PIN);

      if (!adjustingMode) {
        if (dtState != clkState) selectedSlot++;
        else selectedSlot--;

        if (selectedSlot < 0) selectedSlot = 4;
        if (selectedSlot > 4) selectedSlot = 0;
      } else {
        // ปรับค่าตำแหน่งมอเตอร์ทีละ 5 step (ปรับให้ช้าลงหรือเร็วขึ้นได้ที่นี่)
        if (dtState != clkState) tempPos += 5;
        else tempPos -= 5;

        if (tempPos < 0) tempPos = 0;
        stepper.moveTo(tempPos);
      }
      redraw = true;
    }
    lastClk = clkState;

    // 3. วิ่ง Stepper ตลอดเวลาที่อยู่ในโหมดปรับ
    if (adjustingMode) {
      stepper.run();
    }

    // 4. อ่านปุ่มกด Rotary (SW)
    if (digitalRead(SW_PIN) == LOW) {
      delay(50);
      if (digitalRead(SW_PIN) == LOW) {
        if (!adjustingMode) {
          if (selectedSlot == 4) {
            // บันทึกค่าลง EEPROM ตามฟังก์ชัน savePositionsToEEPROM() ที่เราสร้างไว้
            savePositionsToEEPROM();
            while (digitalRead(SW_PIN) == LOW);
            break; // ออกจากฟังก์ชัน
          } else {
            adjustingMode = true;
            tempPos = positions[selectedSlot];
            stepper.moveTo(tempPos);
            redraw = true;
          }
        } else {
          // ออกจากโหมดปรับค่า และบันทึกค่าลง Array รอไว้
          positions[selectedSlot] = tempPos;
          adjustingMode = false;
          redraw = true;
        }

        while (digitalRead(SW_PIN) == LOW) {
          if (adjustingMode) stepper.run();
        }
      }
    }

    // 5. อ่านปุ่ม STOP / BACK
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        if (adjustingMode) {
          adjustingMode = false; // ยกเลิกการปรับค่า ย้อนกลับไปโหมดเลือกช่อง
          redraw = true;
        } else {
          break; // ออกจากฟังก์ชันโดยไม่มีการเซฟ
        }
        while (digitalRead(STOP_BTN_PIN) == LOW);
      }
    }
  }

  pauseMotor();
  u8g2.clearBuffer();
  lastCursorIndex = -1; // บังคับให้วาดเมนูหลักใหม่เพื่อรีเฟรชหน้าจอ
}


void unLoadSorter() {

  Serial.println("Start Homing...");

  // ให้แน่ใจว่าเปิดไฟเลี้ยงมอเตอร์ก่อน Homing
  digitalWrite(enPin, LOW);

  // 1. ตั้งความเร็วให้วิ่งไปหาลิมิตสวิตช์
  stepper.setSpeed(400);

  stepper.runSpeed();
  delay(5000);

  // 3. เมื่อชนสวิตช์ (หลุดลูป) ให้ "บังคับหยุดแบบเด็ดขาด"
  stepper.setSpeed(0);             // รีเซ็ตความเร็ว

  // 4. ตัดไฟมอเตอร์เพื่อป้องกันมอเตอร์ล็อคค้างและร้อน (แกนจะหมุนอิสระได้)
  pauseMotor();

}
