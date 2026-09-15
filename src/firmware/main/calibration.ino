void setActuatorStroke() {
  showActionMessage("Actuator Stroke...");
  delay(1500);
}

void setSorterOffset() {
  int stepMove = 50;
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
        if (dtState != clkState) tempPos += stepMove;
        else tempPos -= stepMove;

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
