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


void updateDisplay() {
  if (currentMenu == 0) drawMenu("MAIN MENU", mainMenu, mainMenuSize);
  else if (currentMenu == 3) drawMenu("CALIBRATION", calMenu, calMenuSize);
  else if (currentMenu == 4) drawMenu("PM MODE", pmMenu, pmMenuSize);
  else if (currentMenu == 5) drawMenu("MODULE FUNCT.", modMenu, modMenuSize);
}

void executeMenuAction() {
  if (currentMenu == 0) {
    // อยู่ใน Main Menu
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
    // อยู่ใน Sub-Menu (ไม่ต้องมี if เช็ค "< Back" แล้ว)
    switch (currentMenu) {

      // ------------------------------------
      // 3. CALIBRATION MENU
      // ------------------------------------
      case 3:
        switch (cursorIndex) {
          case 0: setSensorST188(); break;
          case 1: setActuatorStroke(); break;
          case 2: setSorterOffset(); break;
          case 3: toggleAutoSortMenu(); break;
          case 4: toggleMuteMenu(); break;
        }
        break;

      // ------------------------------------
      // 4. PM MENU
      // ------------------------------------
      case 4:
        switch (cursorIndex) {
          case 0: runIOTesting(); break;
          case 1: runManualJogging(); break;
          case 2: runCommunicationTesting(); break;
          case 3: runDryRun(2); break;
        }
        break;

      // ------------------------------------
      // 5. MODULE FUNCTION MENU
      // ------------------------------------
      case 5:
        switch (cursorIndex) {
          case 0: runDetectPart(1); break;
          case 1: runAlignPart(); break;
          case 2: retractPart(); break;
          case 3: extendPart(1); retractPart(); break;
          case 4: extendPart(2); retractPart(); break;
          case 5: extendPart(3); retractPart(); break;
          case 6: runTrigWaitTMX(1); break;
          case 7: runTransitionPush(1); break;
          case 8: runSortExecute('t'); break;
        }
        break;
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
// 1. แสดงผลสถานะหลัก 3 บรรทัด (PKG, Item No, Status)
void printStatus(String pkgInfo, String itemInfo, String statusInfo) {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(4, 15);
  u8g2.print(pkgInfo);

  u8g2.setCursor(4, 32);
  u8g2.print(itemInfo);

  u8g2.setFont(u8g2_font_7x13B_tf);
  u8g2.setCursor(4, 55);
  u8g2.print(statusInfo);

  u8g2.sendBuffer();
}

// 2. ฟังก์ชันพิเศษสำหรับโชว์คำว่า OK หรือ NG ตัวใหญ่ๆ กลางจอ พร้อมกรอบ
void printBigResult(String resultText) {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawFrame(0, 0, 128, 64);      // วาดกรอบเต็มจอ
  u8g2.drawBox(0, 0, 128, 16);        // แถบหัวข้อทึบด้านบน

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(20, 12);
  u8g2.setDrawColor(0);               // ตัวหนังสือสีดำบนแถบขาว
  u8g2.print("MEASUREMENT");

  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_t0_13b_tf); // ใช้ฟอนต์ขนาดใหญ่พิเศษ
  u8g2.setCursor(35, 45);             // จัดตำแหน่งให้อยู่ตรงกลางจอพอดี
  u8g2.print(resultText);

  u8g2.sendBuffer();
}

void printBigResult2(String resultText) {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawFrame(0, 0, 128, 64);      // วาดกรอบเต็มจอ
  u8g2.drawBox(0, 0, 128, 16);        // แถบหัวข้อทึบด้านบน

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(20, 12);
  u8g2.setDrawColor(0);               // ตัวหนังสือสีดำบนแถบขาว
  u8g2.print("MEASUREMENT");

  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_7x14B_tf); // ใช้ฟอนต์ขนาดใหญ่พิเศษ
  u8g2.setCursor(20, 45);             // จัดตำแหน่งให้อยู่ตรงกลางจอพอดี
  u8g2.print(resultText);

  u8g2.sendBuffer();
}
