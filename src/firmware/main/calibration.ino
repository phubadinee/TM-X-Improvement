void setActuatorStroke() {
  int stepMovePWM = 30; 
  int selectedItem = 0; // 0=Retract, 1=Short, 2=Mid, 3=Long, 4=Save & Exit
  bool adjustingMode = false;
  
  int tempPWM = servoPWM; 
  int currentFeedback = readPositionSmoothly();

  int scrollOffsetAdj = 0;
  const int maxVisibleAdj = 4; // โชว์หน้าจอทีละ 4 บรรทัด
  int lastClk = digitalRead(CLK_PIN);
  bool redraw = true;
  unsigned long lastUpdateFB = 0;

  // เมนูตั้งค่า 5 บรรทัด
  const char* strokeMenu[] = {"1. Retract Pos", "2. Ext. Short", "3. Ext. Mid", "4. Ext. Long", "Save & Exit"};
  int strokeValues[] = {POS_RETRACTED, POS_EXTEND_SHORT, POS_EXTEND_MID, POS_EXTEND_LONG};

  while (true) {

    checkEmergencyReboot();
    
    if (redraw) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tf);

      if (!adjustingMode) {
        // --- โหมดเลือกหัวข้อ ---
        u8g2.setDrawColor(1);
        u8g2.setCursor(2, 9);
        u8g2.print("ADJUST ACTUATOR");
        u8g2.drawLine(0, 12, 128, 12);

        if (selectedItem >= scrollOffsetAdj + maxVisibleAdj) scrollOffsetAdj = selectedItem - maxVisibleAdj + 1;
        else if (selectedItem < scrollOffsetAdj) scrollOffsetAdj = selectedItem;

        for (int i = 0; i < maxVisibleAdj; i++) {
          int itemIndex = scrollOffsetAdj + i;
          if (itemIndex > 4) break; 
          int yPos = 24 + (i * 12);

          if (itemIndex == selectedItem) {
            u8g2.setDrawColor(1);
            u8g2.drawBox(0, yPos - 9, 128, 11);
            u8g2.setDrawColor(0);
          } else {
            u8g2.setDrawColor(1);
          }

          u8g2.setCursor(4, yPos);
          u8g2.print(strokeMenu[itemIndex]);
          
          if (itemIndex < 4) {
            u8g2.print(": ");
            u8g2.print(strokeValues[itemIndex]);
          }
        }
      } else {
        // --- โหมดกำลังปรับค่าด้วยมือ ---
        currentFeedback = readPositionSmoothly();
        u8g2.setDrawColor(1);
        u8g2.drawFrame(0, 0, 128, 64);
        u8g2.drawBox(0, 0, 128, 18);
        u8g2.setDrawColor(0);
        u8g2.setCursor(13, 13);
        u8g2.print(">>> ADJUSTING <<<");

        u8g2.setDrawColor(1);
        u8g2.setCursor(10, 35);
        u8g2.print(strokeMenu[selectedItem]); 

        u8g2.setCursor(10, 50);
        u8g2.print("Set FB to: ");
        u8g2.print(currentFeedback);
      }
      u8g2.sendBuffer();
      redraw = false;
    }

    if (adjustingMode && millis() - lastUpdateFB > 150) {
      redraw = true;
      lastUpdateFB = millis();
    }

    int clkState = digitalRead(CLK_PIN);
    if (clkState != lastClk && clkState == HIGH) {
      if (millis() - lastRotaryTime > 5) { 
        int dtState = digitalRead(DT_PIN);

        if (!adjustingMode) {
          if (dtState != clkState) selectedItem++;
          else selectedItem--;
          if (selectedItem < 0) selectedItem = 4;
          if (selectedItem > 4) selectedItem = 0;
          redraw = true;
        } else {
          if (dtState != clkState) tempPWM += stepMovePWM;
          else tempPWM -= stepMovePWM;

          if (tempPWM < 900) tempPWM = 900;
          if (tempPWM > 2100) tempPWM = 2100;
          actuator.writeMicroseconds(tempPWM); 
        }
        lastRotaryTime = millis();
      }
    }
    lastClk = clkState;

    if (digitalRead(SW_PIN) == LOW) {
      delay(50);
      if (digitalRead(SW_PIN) == LOW) {
        if (!adjustingMode) {
          if (selectedItem == 4) { 
            // "Save & Exit"
            POS_RETRACTED = strokeValues[0];
            POS_EXTEND_SHORT = strokeValues[1];
            POS_EXTEND_MID = strokeValues[2];
            POS_EXTEND_LONG = strokeValues[3];
            
            saveAllToEEPROM();
            
            showActionMessage("  Stroke Saved!");
            for_beep_fast();
            delay(1000);
            while (digitalRead(SW_PIN) == LOW);
            break; 
          } else {
            // ========================================================
            // ฟีเจอร์ใหม่: วิ่งไปหาตำแหน่งล่าสุดอัตโนมัติ ก่อนให้ปรับจูนต่อ
            // ========================================================
            int targetFB = strokeValues[selectedItem];
            
            // แสดงหน้าจอ "กำลังวิ่งไปที่ตำแหน่งเดิม"
            u8g2.clearBuffer();
            u8g2.setDrawColor(1);
            u8g2.drawFrame(0, 0, 128, 64);
            u8g2.drawBox(0, 0, 128, 18);
            u8g2.setDrawColor(0);
            u8g2.setCursor(13, 13);
            u8g2.print(">>> MOVING <<<");
            u8g2.setDrawColor(1);
            u8g2.setCursor(10, 35);
            u8g2.print("Seeking Saved Pos:");
            u8g2.setCursor(10, 50);
            u8g2.print(targetFB);
            u8g2.sendBuffer();

            unsigned long moveStartTime = millis();
            
            // ลูปค้นหาตำแหน่ง (ให้เวลาสูงสุด 5 วินาที ป้องกันมอเตอร์ค้าง)
            while (millis() - moveStartTime < 5000) {
              int currentFB = readPositionSmoothly();
              
              // ถ้าระยะคลาดเคลื่อนไม่เกิน 5 แต้ม ถือว่าถึงที่หมายแล้ว
              if (abs(currentFB - targetFB) <= 5) break; 
              
              if (currentFB > targetFB + 2) { 
                if (servoPWM < 2100) servoPWM += 5; // เพิ่ม PWM เพื่อยืด
              } else if (currentFB < targetFB - 2) {
                if (servoPWM > 900) servoPWM -= 5;  // ลด PWM เพื่อหด
              }
              actuator.writeMicroseconds(servoPWM);
              delay(10);
              
              // ดักจับปุ่มกด (ถ้าอยากยกเลิกกลางคัน ให้กดปุ่ม Rotary หรือปุ่ม Back)
              if (digitalRead(STOP_BTN_PIN) == LOW || digitalRead(SW_PIN) == LOW) {
                delay(100);
                while(digitalRead(STOP_BTN_PIN) == LOW || digitalRead(SW_PIN) == LOW);
                break;
              }
            }

            // เมื่อวิ่งมาถึงที่หมายเสร็จแล้ว ก็เข้าโหมด Adjust ตามปกติ
            adjustingMode = true;
            tempPWM = servoPWM; // ดึงค่า PWM ที่เพิ่งวิ่งมาถึงไปใช้ต่อ
            redraw = true;
          }
        } else {
          // กดยืนยันการปรับค่าด้วยมือ
          currentFeedback = readPositionSmoothly();
          strokeValues[selectedItem] = currentFeedback; 
          servoPWM = tempPWM; 
          
          adjustingMode = false;
          redraw = true;
        }
        while (digitalRead(SW_PIN) == LOW);
      }
    }

    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        if (adjustingMode) {
          adjustingMode = false; 
          redraw = true;
        } else {
          break; 
        }
        while (digitalRead(STOP_BTN_PIN) == LOW);
      }
    }
  }

  u8g2.clearBuffer();
  lastCursorIndex = -1;
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

    checkEmergencyReboot();
    
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
            saveAllToEEPROM();
            while (digitalRead(SW_PIN) == LOW);
            showActionMessage("  Sorter Saved!");
            for_beep_fast();
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

void toggleAutoSortMenu() {
  bool tempState = autoSortEnabled;
  int lastClk = digitalRead(CLK_PIN);
  bool redraw = true;

  while (true) {

    checkEmergencyReboot();
    
    if (redraw) {
      u8g2.clearBuffer();
      u8g2.setDrawColor(1);
      u8g2.drawFrame(0, 0, 128, 64);
      u8g2.drawBox(0, 0, 128, 18);
      u8g2.setDrawColor(0);
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(15, 13);
      u8g2.print("AUTO SORT MODE");

      u8g2.setDrawColor(1);
      u8g2.setFont(u8g2_font_8x13B_tf);
      
      if (tempState) {
        u8g2.setCursor(35, 40);
        u8g2.print("[ ON ]");
      } else {
        u8g2.setCursor(30, 40);
        u8g2.print("[ OFF ]");
      }

      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(15, 58);
      u8g2.print("Press SW to Save");

      u8g2.sendBuffer();
      redraw = false;
    }

    // หมุนเพื่อสลับ ON/OFF
    int clkState = digitalRead(CLK_PIN);
    if (clkState != lastClk && clkState == HIGH) {
      tempState = !tempState; // สลับค่า
      redraw = true;
    }
    lastClk = clkState;

    // กดปุ่มเพื่อบันทึก
    if (digitalRead(SW_PIN) == LOW) {
      delay(50);
      if (digitalRead(SW_PIN) == LOW) {
        autoSortEnabled = tempState;
        
        // ========================================================
        // [แก้ไขตรงนี้] บันทึก Flag ก่อน แล้วค่อยบันทึกค่าที่ Address ถัดไป
        EEPROM.update(EEPROM_ADDR_AUTOSORT, EEPROM_INIT_FLAG); 
        EEPROM.put(EEPROM_ADDR_AUTOSORT + 1, autoSortEnabled); 
        // ========================================================
        
        showActionMessage("  Setting Saved!");
        for_beep_fast();
        while (digitalRead(SW_PIN) == LOW);
        break;
      }
    }

    // กด STOP เพื่อยกเลิก
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        while (digitalRead(STOP_BTN_PIN) == LOW);
        break;
      }
    }
  }
  
  u8g2.clearBuffer();
  lastCursorIndex = -1;
}


void setSensorST188() {
  int step = 0; // 0 = รออ่านค่าตอนไม่มีของ, 1 = รออ่านค่าตอนมีของ
  int valEmpty = 0;
  int valPart = 0;
  
  while (true) {

    checkEmergencyReboot();
    
    int currentVal = read_st188(); // อ่านค่าเรียลไทม์ (0-100)

    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.drawBox(0, 0, 128, 18);
    u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_6x10_tf);
    
    const char* title = "CALIBRATE ST188";
    int titleX = (128 - u8g2.getStrWidth(title)) / 2;
    u8g2.drawStr(titleX, 13, title);

    u8g2.setDrawColor(1);
    
    if (step == 0) {
      u8g2.drawStr(10, 35, "1. Clear Sensor"); // สเต็ป 1: เคลียร์แท่น
    } else {
      u8g2.drawStr(10, 35, "2. Place Part");   // สเต็ป 2: วางชิ้นงาน
    }

    u8g2.setCursor(10, 50);
    u8g2.print("Live Value: ");
    u8g2.print(currentVal);

    u8g2.sendBuffer();

    // --- ตรวจจับปุ่มกดยืนยัน (SW) ---
    if (digitalRead(SW_PIN) == LOW) {
      delay(50);
      if (digitalRead(SW_PIN) == LOW) {
        if (step == 0) {
          valEmpty = currentVal; // บันทึกค่าแท่นเปล่า
          step = 1;              // ขยับไปสเต็ป 2
          for_beep_fast();
        } else {
          valPart = currentVal;  // บันทึกค่ามีของ
          
          // คำนวณค่ากึ่งกลาง (Threshold)
          st188Threshold = (valEmpty + valPart) / 2; 
          saveAllToEEPROM();   // บันทึกลง EEPROM
          
          showActionMessage("  Threshold Saved!");
          for_beep_fast();
          delay(1000);
          while(digitalRead(SW_PIN) == LOW);
          break; // ออกจากฟังก์ชัน
        }
        while(digitalRead(SW_PIN) == LOW);
      }
    }

    // --- ตรวจจับปุ่มยกเลิก (STOP/BACK) ---
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        while (digitalRead(STOP_BTN_PIN) == LOW);
        break; // ออกจากฟังก์ชันโดยไม่เซฟ
      }
    }
    
    delay(50); // กันจอภาพกระพริบรัวเกินไป
  }
  
  u8g2.clearBuffer();
  lastCursorIndex = -1;
}
