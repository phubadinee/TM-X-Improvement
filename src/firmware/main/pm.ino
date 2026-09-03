void runManualJogging() {
  showActionMessage("Manual Jogging...");
  delay(1500);
}

void runIOTesting() {
  showActionMessage("I/O Testing...");
  delay(1500);
}

void runDryRun() {
  showActionMessage("Dry Run Mode...");
  delay(1500);
}

void showPiMessage(const char* msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1309_PIXEL_ON);

  display.setCursor(10, 15);
  display.println("--- MSG FROM PI ---");

  display.setCursor(10, 35);
  display.print(msg);

  display.display();
}

void runPiMonitor() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1309_PIXEL_ON);
  display.setCursor(10, 15);
  display.println("--- PI MONITOR ---");
  display.setCursor(10, 35);
  display.print("Waiting for data...");
  display.display();

  // Wait until SW is released if it was just pressed
  while(digitalRead(SW_PIN) == LOW) {
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
        while(digitalRead(SW_PIN) == LOW) {
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
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1309_PIXEL_ON);
        display.setCursor(10, 15);
        display.println("--- PI MONITOR ---");
        display.setCursor(10, 35);
        display.print(dataFromPi.c_str());
        display.display();
      }
    }
  }

  // Go to home menu when exiting
  currentMenu = 0;
  cursorIndex = 0;
  scrollOffset = 0;
}
