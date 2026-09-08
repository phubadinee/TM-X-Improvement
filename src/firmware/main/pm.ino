void runManualJogging() {
  showActionMessage("Manual Jogging...");
  delay(1500);
}

void runIOTesting() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setDrawColor(1);
  u8g2.setCursor(10, 15);
  u8g2.print("--- IO Testing ---");
  u8g2.sendBuffer();

  while (1) {
    int st188_val = analogRead(st188Pin);
    int st188_val_map = map(st188_val, 0, 1023, 0, 100);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setDrawColor(1);
    
    // วาดหัวข้อค้างไว้
    u8g2.setCursor(10, 15);
    u8g2.print("--- IO Testing ---");

    // แสดงค่าเซนเซอร์
    u8g2.setCursor(10, 35);
    u8g2.print("Val: ");
    u8g2.print(st188_val);
    u8g2.print("    "); // เคาะเว้นวรรคเพื่อเคลียร์ตัวเลขเก่าที่อาจยาวกว่า
    
    u8g2.sendBuffer();  
    delay(100); // หน่วงเวลาเล็กน้อยเพื่อไม่ให้อัปเดตหน้าจอเร็วเกินไปจนกระพริบ
  }
}

void runDryRun() {
  showActionMessage("Dry Run Mode...");
  delay(1500);
}

void showPiMessage(const char* msg) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setDrawColor(1);

  u8g2.setCursor(10, 15);
  u8g2.print("--- MSG FROM PI ---");

  u8g2.setCursor(10, 35);
  u8g2.print(msg);

  u8g2.sendBuffer();
}

void runPiMonitor() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setDrawColor(1);
  
  u8g2.setCursor(10, 15);
  u8g2.print("--- PI MONITOR ---");
  
  u8g2.setCursor(10, 35);
  u8g2.print("Waiting for data...");
  u8g2.sendBuffer();

  // Wait until SW is released if it was just pressed
  while (digitalRead(SW_PIN) == LOW) {
    delay(10);
  }
  delay(100);

  bool exitMonitor = false;
  while (!exitMonitor) {
    // Check if SW_PIN is pressed to exit
    if (digitalRead(SW_PIN) == LOW) {
      delay(50); // debounce
      if (digitalRead(SW_PIN) == LOW) {
        exitMonitor = true;
        // Wait until SW is released
        while (digitalRead(SW_PIN) == LOW) {
          delay(10);
        }
      }
    }

    // Check for serial data
    if (Serial.available()) {
      String dataFromPi = Serial.readStringUntil('\n');
      dataFromPi.trim();

      if (dataFromPi.length() > 0) {
        // Send a response back to Pi
        Serial.print("Mega processed command: [");
        Serial.print(dataFromPi);
        Serial.println("]");

        // Update display
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setDrawColor(1);
        
        u8g2.setCursor(10, 15);
        u8g2.print("--- PI MONITOR ---");
        
        u8g2.setCursor(10, 35);
        u8g2.print(dataFromPi.c_str());
        
        u8g2.sendBuffer();
      }
    }
  }

  // Go to home menu when exiting
  currentMenu = 0;
  cursorIndex = 0;
  scrollOffset = 0;
}