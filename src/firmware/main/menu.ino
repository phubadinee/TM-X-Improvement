void handleRotaryMenu() {
  currentStateCLK = digitalRead(CLK_PIN);

  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    if ((millis() - lastRotaryTime) > debounceDelay) {
      if (digitalRead(DT_PIN) != currentStateCLK) {
        cursorIndex++;
      } else {
        cursorIndex--;
      }

      // กำหนดขอบเขตสูงสุดต่ำสุดตามเมนูปัจจุบัน
      int maxItems = 0;
      if (currentMenu == 0) maxItems = mainMenuSize;
      else if (currentMenu == 3) maxItems = calMenuSize;  // Calibration
      else if (currentMenu == 4) maxItems = pmMenuSize;   // PM
      else if (currentMenu == 5) maxItems = modMenuSize;  // Module Function

      if (cursorIndex >= maxItems) cursorIndex = maxItems - 1;
      if (cursorIndex < 0) cursorIndex = 0;

      lastRotaryTime = millis();
    }
  }
  lastStateCLK = currentStateCLK;
}

// ==========================================
// ฟังก์ชันวาดเมนูด้วย U8g2 (Highlight & Scrollbar)
// ==========================================
void drawMenu(const char* title, const char* items[], int itemCount) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  // 1. วาดหัวข้อเมนู
  u8g2.setDrawColor(1);
  u8g2.setCursor(2, 9);
  u8g2.print(title);
  
  // เส้นคั่นหัวข้อแบบเต็มจอ
  u8g2.drawLine(0, 12, 128, 12);

  // คำนวณการเลื่อนหน้าจอแบบซ่อน Scrollbar
  if (cursorIndex >= scrollOffset + maxVisibleItems) {
    scrollOffset = cursorIndex - maxVisibleItems + 1;
  } else if (cursorIndex < scrollOffset) {
    scrollOffset = cursorIndex;
  }

  // 2. วาดรายการเมนู
  for (int i = 0; i < maxVisibleItems; i++) {
    int itemIndex = scrollOffset + i;
    if (itemIndex >= itemCount) break;

    int yPos = 24 + (i * 12);

    if (itemIndex == cursorIndex) {
      // แถบ Highlight สีขาว วาดเต็มความกว้างจอ (128 px)
      u8g2.setDrawColor(1);
      u8g2.drawBox(0, yPos - 9, 128, 11);
      
      // ตัวหนังสือสีดำบนแถบขาว
      u8g2.setDrawColor(0); 
    } else {
      // ตัวหนังสือปกติสีขาว
      u8g2.setDrawColor(1); 
    }

    u8g2.setCursor(4, yPos);
    u8g2.print(items[itemIndex]);
  }

  u8g2.sendBuffer();
}


void drawMenu2(const char* title, const char* items[], int itemCount) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.setDrawColor(1);
  u8g2.setCursor(0, 9);
  u8g2.print("--- ");
  u8g2.print(title);
  u8g2.print(" ---");
  
  u8g2.drawLine(0, 12, 128, 12);

  if (cursorIndex >= scrollOffset + maxVisibleItems) {
    scrollOffset = cursorIndex - maxVisibleItems + 1;
  } else if (cursorIndex < scrollOffset) {
    scrollOffset = cursorIndex;
  }

  for (int i = 0; i < maxVisibleItems; i++) {
    int itemIndex = scrollOffset + i;
    if (itemIndex >= itemCount) break;

    int yPos = 25 + (i * 12);

    if (itemIndex == cursorIndex) {
      u8g2.setDrawColor(1);
      u8g2.drawBox(0, yPos - 9, 128, 11);
      u8g2.setDrawColor(0); 
    } else {
      u8g2.setDrawColor(1); 
    }

    u8g2.setCursor(4, yPos);
    u8g2.print(items[itemIndex]);
  }

  u8g2.sendBuffer();
}

void updateDisplay() {
  if (currentMenu == 0) drawMenu("MAIN MENU", mainMenu, mainMenuSize);
  else if (currentMenu == 3) drawMenu("CALIBRATION", calMenu, calMenuSize);
  else if (currentMenu == 4) drawMenu("PM MODE", pmMenu, pmMenuSize);
  else if (currentMenu == 5) drawMenu("MODULE FUNCT.", modMenu, modMenuSize);
}

void executeMenuAction() {
  if (currentMenu == 0) {
    // ตรวจสอบว่ากดเมนูสั่งงานตรงๆ หรือเข้า Sub-Menu
    if (cursorIndex == 0) {
      runStart();
    } else if (cursorIndex == 1) {
      runSystemHoming();
    } else {
      // เข้า Sub-Menu (เช่น Index 2 ไป CurrentMenu 3)
      currentMenu = cursorIndex + 1;
      cursorIndex = 0;
      scrollOffset = 0;
    }
  } else {
    // อยู่ใน Sub-Menu
    if (cursorIndex == 0) {
      // "< Back" ให้กลับไป Main Menu
      currentMenu = 0;
      cursorIndex = 0;
      scrollOffset = 0;
    } else {
      switch (currentMenu) {

        // ------------------------------------
        // 3. CALIBRATION MENU
        // ------------------------------------
        case 3:
          switch (cursorIndex) {
            case 1: setActuatorStroke(); break;
            case 2: setSorterOffset(); break;
            case 3: setServoTransition(); break;
          }
          break;

        // ------------------------------------
        // 4. PM MENU
        // ------------------------------------
        case 4:
          switch (cursorIndex) {
            case 1: runManualJogging(); break;
            case 2: runIOTesting(); break;
            case 3: runCommunicationTesting(); break;
            case 4: runDryRun(); break;
          }
          break;

        // ------------------------------------
        // 5. MODULE FUNCTION MENU
        // ------------------------------------
        case 5:
          switch (cursorIndex) {
            case 1: runDetectPart(); break;
            case 2: runAlignPart(); break;
            case 3: runTrigWaitTMX(); break;
            case 4: runTransitionPush(); break;
            case 5: runSortExecute(); break;
          }
          break;
      }
    }
  }
  updateDisplay();
}


void showActionMessage(const char* actionName) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // วาดกรอบสี่เหลี่ยมเต็มขอบหน้าจอ (128x64)
  u8g2.setDrawColor(1);
  u8g2.drawFrame(0, 0, 128, 64);
  
  // วาดแถบทึบด้านบนเป็น Header แบบเต็มจอ
  u8g2.drawBox(0, 0, 128, 18); 

  // ข้อความ Header สีดำทับพื้นขาว (Invert)
  u8g2.setDrawColor(0);
  u8g2.setCursor(13, 13);
  u8g2.print(">>> EXECUTING <<<");

  // ชื่อฟังก์ชันที่กำลังรัน (ตัวหนังสือสีขาวปกติ ตรงกลางจอ)
  u8g2.setDrawColor(1);
  u8g2.setCursor(10, 40); // ปรับตำแหน่งแกน X, Y ให้พอดี
  u8g2.print(actionName);
  u8g2.sendBuffer();
}

void showActionMessage2(const char* actionName) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setDrawColor(1);

  u8g2.setCursor(10, 25);
  u8g2.print("--- EXECUTING ---");

  u8g2.setCursor(10, 45);
  u8g2.print(actionName);

  u8g2.sendBuffer();
}
