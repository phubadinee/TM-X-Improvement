void setActuatorStroke() {
  int stepMovePWM = 20; // การขยับ PWM ต่อ 1 คลิก (ปรับให้ขยับเร็ว/ช้าได้ที่นี่)
  int selectedItem = 0; // 0=Retract, 1=Slow Start, 2=Extend, 3=Save & Exit
  bool adjustingMode = false;
  
  int tempPWM = servoPWM; // ดึงค่า PWM ปัจจุบันมาใช้เป็นจุดเริ่มต้น
  int currentFeedback = readPositionSmoothly();

  int scrollOffsetAdj = 0;
  const int maxVisibleAdj = 4;
  int lastClk = digitalRead(CLK_PIN);
  bool redraw = true;
  unsigned long lastUpdateFB = 0;

  // เมนูและอาเรย์พักค่าชั่วคราว
  const char* strokeMenu[] = {"1. Retract Pos", "2. Slow Start Pos", "3. Extend Pos", "Save & Exit"};
  int strokeValues[] = {POS_RETRACTED, POS_SLOW_START, POS_EXTENDED};

  while (true) {
    // 1. จัดการวาดหน้าจอ
    if (redraw) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tf);

      if (!adjustingMode) {
        // ==========================================
        // โหมดเลือกหัวข้อที่จะตั้งค่า
        // ==========================================
        u8g2.setDrawColor(1);
        u8g2.setCursor(2, 9);
        u8g2.print("ADJUST ACTUATOR");
        u8g2.drawLine(0, 12, 128, 12);

        // คำนวณการเลื่อนหน้าจอ
        if (selectedItem >= scrollOffsetAdj + maxVisibleAdj) scrollOffsetAdj = selectedItem - maxVisibleAdj + 1;
        else if (selectedItem < scrollOffsetAdj) scrollOffsetAdj = selectedItem;

        for (int i = 0; i < maxVisibleAdj; i++) {
          int itemIndex = scrollOffsetAdj + i;
          if (itemIndex > 3) break;

          int yPos = 24 + (i * 12);

          if (itemIndex == selectedItem) {
            u8g2.setDrawColor(1);
            u8g2.drawBox(0, yPos - 9, 128, 11);
            u8g2.setDrawColor(0); // ตัวอักษรสีดำบนแถบขาว
          } else {
            u8g2.setDrawColor(1); // ตัวอักษรสีขาว
          }

          u8g2.setCursor(4, yPos);
          u8g2.print(strokeMenu[itemIndex]);
          
          // โชว์ค่า Feedback ปัจจุบันที่ถูกตั้งไว้
          if (itemIndex < 3) {
            u8g2.print(": ");
            u8g2.print(strokeValues[itemIndex]);
          }
        }
      } else {
        // ==========================================
        // โหมดกำลังหมุนปรับระยะ (โชว์ PWM และ Feedback สดๆ)
        // ==========================================
        currentFeedback = readPositionSmoothly();

        u8g2.setDrawColor(1);
        u8g2.drawFrame(0, 0, 128, 64);
        u8g2.drawBox(0, 0, 128, 18);

        u8g2.setDrawColor(0);
        u8g2.setCursor(13, 13);
        u8g2.print(">>> ADJUSTING <<<");

        u8g2.setDrawColor(1);
        u8g2.setCursor(10, 35);
        u8g2.print(strokeMenu[selectedItem]); // โชว์ว่ากำลังปรับตัวไหนอยู่

        u8g2.setCursor(10, 50);
        u8g2.print("PWM: ");
        u8g2.print(tempPWM);
        u8g2.print(" FB: ");
        u8g2.print(currentFeedback);
      }
      u8g2.sendBuffer();
      redraw = false;
    }

    // เพื่อให้จออัปเดตค่า Feedback (FB) วิ่งตามจริงแบบ Realtime ตอนขยับ Actuator
    if (adjustingMode && millis() - lastUpdateFB > 100) {
      redraw = true;
      lastUpdateFB = millis();
    }

    // 2. อ่านค่า Rotary Encoder
    int clkState = digitalRead(CLK_PIN);
    if (clkState != lastClk && clkState == HIGH) {
      int dtState = digitalRead(DT_PIN);

      if (!adjustingMode) {
        // เลื่อนขึ้น-ลง ในเมนู
        if (dtState != clkState) selectedItem++;
        else selectedItem--;

        if (selectedItem < 0) selectedItem = 3;
        if (selectedItem > 3) selectedItem = 0;
      } else {
        // โหมดปรับค่า: หมุนเพื่อปรับค่า PWM สั่งมอเตอร์ยืด/หด
        if (dtState != clkState) tempPWM += stepMovePWM; // หมุนขวา
        else tempPWM -= stepMovePWM;                     // หมุนซ้าย

        // ป้องกันค่า PWM เกินมาตรฐาน (ส่วนใหญ่ 1000 - 2000)
        if (tempPWM < 900) tempPWM = 900;
        if (tempPWM > 2100) tempPWM = 2100;
        
        // สั่งขยับ Actuator ทันที
        actuator.writeMicroseconds(tempPWM);
      }
      redraw = true;
    }
    lastClk = clkState;

    // 3. อ่านปุ่มกด (SW)
    if (digitalRead(SW_PIN) == LOW) {
      delay(50);
      if (digitalRead(SW_PIN) == LOW) {
        if (!adjustingMode) {
          if (selectedItem == 3) {
            // "Save & Exit" -> บันทึกค่าลงตัวแปรหลัก และลง EEPROM
            POS_RETRACTED = strokeValues[0];
            POS_SLOW_START = strokeValues[1];
            POS_EXTENDED = strokeValues[2];
            
            saveActuatorToEEPROM(); // บันทึกลง EEPROM
            
            showActionMessage("Saved!"); // โชว์ข้อความว่าเซฟเสร็จแล้ว
            delay(1000);
            while (digitalRead(SW_PIN) == LOW);
            break; // ออกจากฟังก์ชัน
          } else {
            // เข้าสู่โหมดปรับค่า
            adjustingMode = true;
            redraw = true;
          }
        } else {
          // กดปุ่มขณะกำลังปรับค่า -> ตกลงใช้ค่า Feedback ปัจจุบัน
          currentFeedback = readPositionSmoothly();
          strokeValues[selectedItem] = currentFeedback; // จำค่าไว้ในอาเรย์ชั่วคราวก่อน
          
          adjustingMode = false; // กลับไปหน้าเลือกเมนู
          redraw = true;
        }

        while (digitalRead(SW_PIN) == LOW);
      }
    }

    // 4. อ่านปุ่ม STOP / BACK
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        if (adjustingMode) {
          adjustingMode = false; // ยกเลิกการปรับค่า ย้อนกลับไปหน้าเลือกเมนู
          redraw = true;
        } else {
          break; // ออกจากฟังก์ชันเลย (ทิ้งค่าที่ตั้งไว้ ไม่ Save)
        }
        while (digitalRead(STOP_BTN_PIN) == LOW);
      }
    }
  }

  u8g2.clearBuffer();
  lastCursorIndex = -1; // บังคับให้เมนูหลักรีเฟรชหน้าจอ
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
