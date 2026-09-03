void handleRotaryMenu() {
  currentStateCLK = digitalRead(CLK_PIN);

  // ตรวจจับเฉพาะการเปลี่ยนจาก LOW ไป HIGH (Rising Edge)
  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {

    // เช็คระยะเวลาว่าห่างจากการหมุนครั้งล่าสุดเกินค่า Debounce หรือไม่
    if ((millis() - lastRotaryTime) > debounceDelay) {

      // ถ้า DT ไม่เหมือนกับ CLK ตอนเปลี่ยน แสดงว่าหมุนขวา (ลงล่าง)
      if (digitalRead(DT_PIN) != currentStateCLK) {
        cursorIndex++;
      } else {
        cursorIndex--;  // หมุนซ้าย (ขึ้นบน)
      }

      // กำหนดขอบเขตสูงสุดต่ำสุดตามเมนูปัจจุบัน
      int maxItems = 0;
      if (currentMenu == 0) maxItems = mainMenuSize;
      else if (currentMenu == 1) maxItems = opMenuSize;
      else if (currentMenu == 2) maxItems = pmMenuSize;
      else if (currentMenu == 3) maxItems = calMenuSize;

      // จำกัดไม่ให้เคอร์เซอร์เลยขอบเมนู
      if (cursorIndex >= maxItems) cursorIndex = maxItems - 1;
      if (cursorIndex < 0) cursorIndex = 0;

      // บันทึกเวลาที่หมุนสำเร็จ
      lastRotaryTime = millis();
    }
  }

  lastStateCLK = currentStateCLK;
}

void drawMenu(const char* title, const char* items[], int itemCount) {
  display.clearDisplay();
  display.setTextSize(1);

  display.setTextColor(SSD1309_PIXEL_ON);
  display.setCursor(0, 0);
  display.print("--- ");
  display.print(title);
  display.println(" ---");
  display.drawLine(0, 10, 128, 10, SSD1309_PIXEL_ON);

  if (cursorIndex >= scrollOffset + maxVisibleItems) {
    scrollOffset = cursorIndex - maxVisibleItems + 1;
  } else if (cursorIndex < scrollOffset) {
    scrollOffset = cursorIndex;
  }

  for (int i = 0; i < maxVisibleItems; i++) {
    int itemIndex = scrollOffset + i;
    if (itemIndex >= itemCount) break;

    int yPos = 16 + (i * 12);

    if (itemIndex == cursorIndex) {
      display.fillRect(0, yPos - 2, 128, 11, SSD1309_PIXEL_ON);
      display.setTextColor(0);  // ตัวอักษรสีดำบนแถบทึบ
    } else {
      display.setTextColor(SSD1309_PIXEL_ON);
    }

    display.setCursor(4, yPos);
    display.print(items[itemIndex]);
  }

  display.display();
}

void updateDisplay() {
  if (currentMenu == 0) drawMenu("MAIN MENU", mainMenu, mainMenuSize);
  else if (currentMenu == 1) drawMenu("OPERATION", opMenu, opMenuSize);
  else if (currentMenu == 2) drawMenu("PM MODE", pmMenu, pmMenuSize);
  else if (currentMenu == 3) drawMenu("CALIBRATION", calMenu, calMenuSize);
}

void executeMenuAction() {
  if (currentMenu == 0) {
    // อยู่ที่ MAIN MENU: กดเพื่อเข้า Sub-Menu ต่างๆ
    currentMenu = cursorIndex + 1;
    cursorIndex = 0;
    scrollOffset = 0;
  } else {
    // อยู่ใน Sub-Menu
    if (cursorIndex == 0) {
      // ตำแหน่งที่ 0 คือคำสั่ง "< Back" ให้กลับไป Main Menu
      currentMenu = 0;
      cursorIndex = 0;
      scrollOffset = 0;
    } else {
      // สั่งงานฟังก์ชันตาม CurrentMenu และ CursorIndex
      switch (currentMenu) {

        // ------------------------------------
        // 1. OPERATION MENU
        // ------------------------------------
        case 1:
          switch (cursorIndex) {
            case 1: runSystemHoming(); break;    // "System Homing"
            case 2: runStart(); break;           // "Start"
            case 3: runDetectPart(); break;      // "Detect Part"
            case 4: runAlignPart(); break;       // "Align Part"
            case 5: runTrigWaitTMX(); break;     // "Trig & Wait TM-X"
            case 6: runTransitionPush(); break;  // "Transition Push"
            case 7: runSortExecute(); break;     // "Sort Execute"
            case 8: runEmergencyHalt(); break;   // "Emergency Halt"
          }
          break;

        // ------------------------------------
        // 2. PM MENU
        // ------------------------------------
        case 2:
          switch (cursorIndex) {
            case 1: runManualJogging(); break;  // "Manual Jogging"
            case 2: runIOTesting(); break;      // "IO Testing"
            case 3: runDryRun(); break;         // "Dry Run"
            case 4: runPiMonitor(); break;      // "Pi Monitor"
          }
          break;

        // ------------------------------------
        // 3. CALIBRATION MENU
        // ------------------------------------
        case 3:
          switch (cursorIndex) {
            case 1: runActuatorStroke(); break;   // "Actuator Stroke"
            case 2: runSorterOffset(); break;     // "Sorter Offset"
            case 3: runServoTransition(); break;  // "Servo Transition"
          }
          break;
      }
    }
  }
  updateDisplay();
}


void showActionMessage(const char* actionName) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1309_PIXEL_ON);

  // จัดข้อความให้อยู่กึ่งกลางคร่าวๆ
  display.setCursor(10, 15);
  display.println("--- EXECUTING ---");

  display.setCursor(10, 35);
  display.print(actionName);

  display.display();
}


void runSystemHoming() {
  showActionMessage("System Homing...");
  delay(1500);
}


void runEmergencyHalt() {
  showActionMessage("! EMERGENCY HALT !");
  delay(2000);  // ให้อ่านนานหน่อย
}
