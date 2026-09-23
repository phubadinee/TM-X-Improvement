void runManualJogging() {
  // ดักรอให้ปล่อยปุ่มก่อนเข้าทำงาน ป้องกันปุ่มลั่น
  while (digitalRead(SW_PIN) == LOW || digitalRead(STOP_BTN_PIN) == LOW) {
    delay(10);
  }
  delay(50);

  int selectedMotor = 0; // 0 = Sorter (Stepper), 1 = Push (Servo), 2 = Actuator

  // ดึงค่าตำแหน่งปัจจุบันของแต่ละมอเตอร์มาตั้งต้น
  long jogStepPos = stepper.currentPosition();
  int jogServoPos = myServo.read(); // อ่านค่าองศาปัจจุบันของ Servo
  int jogLinPos = servoPWM;         // ค่า PWM ปัจจุบันของ Actuator

  int lastClk = digitalRead(CLK_PIN);
  bool redraw = true;

  // เปิดไฟเลี้ยง Stepper เผื่อไว้ให้พร้อมขยับ
  digitalWrite(enPin, LOW);

  // ล้างหน้าจอก่อนเข้าลูป
  u8g2.clearBuffer();
  u8g2.sendBuffer();

  while (true) {
    // ----------------------------------------------------
    // 1. อ่านค่า Rotary Encoder เพื่อสั่งมอเตอร์ขยับ
    // ----------------------------------------------------
    int clkState = digitalRead(CLK_PIN);
    if (clkState != lastClk && clkState == HIGH) {
      int dtState = digitalRead(DT_PIN);
      int dir = (dtState != clkState) ? 1 : -1; // 1 = หมุนขวา, -1 = หมุนซ้าย

      // ควบคุม Stepper (Sorter) -> ขยับทีละ 10 สเต็ป
      if (selectedMotor == 0) {
        jogStepPos += (dir * 50);
        if (jogStepPos < 0) jogStepPos = 0;     // ล็อคขอบเขตหดสุด (ปรับได้)
        if (jogStepPos > 2500) jogStepPos = 2500;
        stepper.moveTo(jogStepPos);
      }
      // ควบคุม Servo (Push) -> ขยับทีละ 5 องศา
      else if (selectedMotor == 1) {
        jogServoPos += (dir * 1);
        if (jogServoPos < 80) jogServoPos = 80;     // ล็อคขอบเขตไม่ให้ต่ำกว่า 0
        if (jogServoPos > 100) jogServoPos = 100; // ล็อคขอบเขตไม่ให้เกิน 180
        myServo.write(jogServoPos);
      }
      // ควบคุม Linear Actuator -> ขยับทีละ 20 PWM (Microseconds)
      else if (selectedMotor == 2) {
        jogLinPos += (dir * 50);
        if (jogLinPos < 700) jogLinPos = 700;     // ล็อคขอบเขตหดสุด (ปรับได้)
        if (jogLinPos > 2200) jogLinPos = 2200;   // ล็อคขอบเขตยืดสุด (ปรับได้)
        actuator.writeMicroseconds(jogLinPos);
        servoPWM = jogLinPos; // อัพเดตค่าตัวแปรหลักด้วย
      }
      redraw = true;
    }
    lastClk = clkState;

    // ต้องสั่ง run() ไว้ในลูปเสมอ เพื่อให้ Stepper หมุนตามมือเราทัน
    stepper.run();

    // ----------------------------------------------------
    // 2. เช็คปุ่มกด Rotary (SW) เพื่อ "สลับมอเตอร์"
    // ----------------------------------------------------
    if (digitalRead(SW_PIN) == LOW) {
      delay(50);
      if (digitalRead(SW_PIN) == LOW) {
        selectedMotor++; // เลื่อนตัวเลือก
        if (selectedMotor > 2) selectedMotor = 0; // วนกลับไป 0 ใหม่
        redraw = true;

        // รอจนกว่าจะปล่อยปุ่ม (ให้มอเตอร์ขยับไปด้วยเผื่อยังวิ่งไม่ถึง)
        while (digitalRead(SW_PIN) == LOW) {
          stepper.run();
        }
        delay(50);
      }
    }

    // ----------------------------------------------------
    // 3. เช็คปุ่ม STOP/BACK เพื่อกลับไปหน้าหลัก
    // ----------------------------------------------------
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        currentMenu = 0;
        cursorIndex = 0;
        scrollOffset = 0;

        while (digitalRead(STOP_BTN_PIN) == LOW) {
          stepper.run();
        }
        runSystemHoming();
        delay(50);
        break; // เด้งออกจากลูป กลับเมนูหลัก
      }
    }

    // ----------------------------------------------------
    // 4. วาดหน้าจอ OLED (อัพเดตเฉพาะตอนมีการหมุนหรือกดสลับ)
    // ----------------------------------------------------
    if (redraw) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tf);

      // --- Header ---
      u8g2.setDrawColor(1);
      u8g2.drawBox(0, 0, 128, 14); // พื้นหลังดำ
      u8g2.setDrawColor(0);        // ตัวหนังสือขาว
      u8g2.setCursor(4, 11);
      u8g2.print("Manual Jogging");
      u8g2.setDrawColor(1);
      u8g2.drawLine(0, 15, 128, 15);

      // --- List of Motors ---
      for (int i = 0; i < 3; i++) {
        int yPos = 28 + (i * 10);

        // ถ้าเป็นมอเตอร์ที่ถูกเลือกอยู่ ให้ทำแถบ Invert สีดำ/ขาว
        if (i == selectedMotor) {
          u8g2.setDrawColor(1);
          u8g2.drawBox(0, yPos - 9, 128, 12);
          u8g2.setDrawColor(0); // ตัวหนังสือดำบนแถบขาว
        } else {
          u8g2.setDrawColor(1); // ตัวหนังสือปกติ
        }

        u8g2.setCursor(5, yPos);

        if (i == 0) {
          u8g2.print("1. Sorter : ");
          u8g2.print(jogStepPos);
        }
        else if (i == 1) {
          u8g2.print("2. Servo  : ");
          u8g2.print(jogServoPos);
          u8g2.print(" deg");
        }
        else if (i == 2) {
          u8g2.print("3. Act.   : ");
          u8g2.print(jogLinPos);
          u8g2.print(" us");
        }
      }

      // คำอธิบายวิธีใช้อยู่บรรทัดล่างสุด
      u8g2.setDrawColor(1);
      u8g2.setCursor(2, 62);
      u8g2.print("[Click] to Switch");

      u8g2.sendBuffer();
      redraw = false; // เคลียร์สถานะการวาด เพื่อไม่ให้จอวาดซ้ำจน Stepper กระตุก
    }
  }

  // ออกจากโหมดนี้แล้ว ตัดไฟ Stepper เพื่อพักมอเตอร์
  pauseMotor();

  // เคลียร์จอและเตรียมให้เมนูหลักวาดใหม่
  u8g2.clearBuffer();
  lastCursorIndex = -1;
}


void runIOTesting() {
  int currentPage = 1;          // ตัวแปรเก็บหน้าปัจจุบัน (1 หรือ 2)
  int lastClk = digitalRead(CLK_PIN);

  // เพิ่มตัวแปรสำหรับจำสถานะเซนเซอร์ (ป้องกัน Buzzer ดังรัวๆ ทุก 50ms)
  bool lastDetectState = false;

  // ล้างหน้าจอก่อนเข้าลูป
  u8g2.clearBuffer();
  u8g2.sendBuffer();

  while (true) {
    // ----------------------------------------------------
    // 1. อ่านค่า Rotary Encoder เพื่อสลับหน้า (Page 1 <-> Page 2)
    // ----------------------------------------------------
    int clkState = digitalRead(CLK_PIN);
    if (clkState != lastClk && clkState == HIGH) {
      int dtState = digitalRead(DT_PIN);
      if (dtState != clkState) {
        currentPage++; // หมุนขวา
      } else {
        currentPage--; // หมุนซ้าย
      }

      // วนลูปหน้าจอ 1 และ 2
      if (currentPage > 2) currentPage = 1;
      if (currentPage < 1) currentPage = 2;
    }
    lastClk = clkState;

    // ----------------------------------------------------
    // 2. เช็คปุ่มกด ถ้ากดปุ่ม SW หรือ STOP ให้เด้งกลับ "หน้าหลัก"
    // ----------------------------------------------------
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50); // กันปุ่มสั่น (Debounce)

      if (digitalRead(STOP_BTN_PIN) == LOW) {
        // --- สั่งให้กลับไปที่เมนูหลัก (Main Menu) ---
        currentMenu = 0;
        cursorIndex = 0;
        scrollOffset = 0;

        // รอจนกว่าผู้ใช้จะปล่อยปุ่ม ป้องกันปุ่มลั่นไปกดเมนูอื่น
        while (digitalRead(STOP_BTN_PIN) == LOW);
        delay(50);

        break; // เด้งออกจากลูป I/O Testing
      }
    }

    // ----------------------------------------------------
    // 3. จัดการวาดหน้าจอ OLED
    // ----------------------------------------------------
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);

    // --- Header ---
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 0, 128, 14); // แถบพื้นหลังดำ
    u8g2.setDrawColor(0);        // ตัวหนังสือขาว (บนพื้นดำ)
    u8g2.setCursor(4, 11);
    u8g2.print("IO Testing");

    // แสดงเลขหน้ามุมขวาบน
    u8g2.setCursor(95, 11);
    u8g2.print("P:");
    u8g2.print(currentPage);
    u8g2.print("/2");

    u8g2.setDrawColor(1); // คืนค่าสีวาดปกติ
    u8g2.drawLine(0, 15, 128, 15);

    // --- Page 1 : ST188 Sensor ---
    if (currentPage == 1) {
      int st188_val = analogRead(st188Pin);
      int st188_val_map = map(st188_val, 0, 1023, 0, 100);

      // ตั้งเกณฑ์สมมติ (แก้ตัวเลขได้): ถ้าค่าน้อยกว่า 80 ถือว่าเจอชิ้นงาน
      bool isDetected = (st188_val_map < st188Threshold);

      u8g2.setCursor(5, 32);
      u8g2.print("ST188 (Raw) : ");
      u8g2.print(st188_val);

      u8g2.setCursor(5, 46);
      u8g2.print("ST188 (Map) : ");
      u8g2.print(st188_val_map);

      u8g2.setCursor(5, 60);
      u8g2.print("Result: ");

      if (isDetected) {
        u8g2.print("[DETECTED]");

        // เช็คว่าเพิ่งตรวจเจอครั้งแรกใช่ไหม? ค่อยส่งเสียงดัง
        if (lastDetectState == false) {
          beepHigh();
          lastDetectState = true; // จำไว้ว่ากำลังเจอของอยู่ จะได้ไม่ดังซ้ำ
        }
      } else {
        u8g2.print("[NO PART]");

        // เช็คว่าเพิ่งเอาของออกใช่ไหม? ค่อยส่งเสียงเตือน
        if (lastDetectState == true) {
          beepLow();
          lastDetectState = false; // รีเซ็ตสถานะ
        }
      }
    }

    // --- Page 2 : Limit Switches & Button ---
    else if (currentPage == 2) {
      // อ่านค่าสถานะ
      int lm_servo_st  = digitalRead(limit_servo);
      int lm_sorter_st = digitalRead(limit_sorter);
      int sw_btn_st    = digitalRead(SW_PIN);

      // วาดสถานะ Limit Servo
      u8g2.setCursor(5, 30);
      u8g2.print("LM Servo  : ");
      u8g2.print(lm_servo_st == HIGH ? "HIGH (1)" : "LOW (0)");

      // วาดสถานะ Limit Sorter
      u8g2.setCursor(5, 44);
      u8g2.print("LM Sorter : ");
      u8g2.print(lm_sorter_st == HIGH ? "HIGH (1)" : "LOW (0)");

      // วาดสถานะ Physical Button (SW)
      u8g2.setCursor(5, 58);
      u8g2.print("SW Button : ");
      u8g2.print(sw_btn_st == HIGH ? "HIGH (1)" : "LOW (0)");
    }

    u8g2.sendBuffer();

    // หน่วงเวลาสั้นๆ เพื่อให้หน้าจอไม่กระพริบ และยังอ่าน Encoder ได้ลื่นไหล
    delay(50);
  }

  // ก่อนออกจากฟังก์ชัน เคลียร์จอ 1 ครั้ง
  u8g2.clearBuffer();
  lastCursorIndex = -1; // บังคับให้เมนูหลักวาดตัวเองใหม่
}

void runDryRun(int set) {
  Serial.println("======= [Start] Dry Run Mode =======");

  for (int i = 0; i < set; i++) {
    
    // อัปเดตหน้าจอ OLED ให้แสดงว่ากำลังทำงานรอบที่เท่าไหร่
    String statusText = "Running " + String(i + 1) + "/" + String(set);
    showActionMessage(statusText.c_str());

    // --- ตรวจจับการกดปุ่ม STOP / BACK ก่อนเริ่มแต่ละรอบ ---
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50); // หน่วงเวลาป้องกันสัญญาณสั่น
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        Serial.println("⚠️ Dry Run Aborted by User!");
        retractPart(); // สั่งดึงก้านมอเตอร์เก็บเพื่อความปลอดภัย
        while (digitalRead(STOP_BTN_PIN) == LOW); 
        break; // กระโดดออกจากลูป for ทันที
      }
    }

    Serial.print("--- Running Set: ");
    Serial.print(i + 1);
    Serial.print(" / ");
    Serial.println(set);

    runDetectPart_TEST();
    
    // ใช้ฟังก์ชันแบบมี _check เพื่อให้เหมือนระบบรันจริง
    if (extendPart_check(1) == -1) {
      retractPart();
      printBigResult2("DRY RUN FAILED");
      delay(1500);
      break; // ถ้าก้านติดขัดตอนเทส ให้ยกเลิก Dry Run ทันที
    }

    runTrigWaitTMX_TEST();

    // ==================================================
    // ตรวจสอบเงื่อนไข Auto Sort (เหมือนตอนรันจริง)
    // ==================================================
    if (autoSortEnabled == true) {
      // โหมดเปิด Auto: หดก้าน, คัดแยก, และผลักชิ้นงาน
      retractPart();
      runSortExecute('t');   // ส่ง 't' ไปเพื่อจำลองการวิ่งไปทุกช่อง
      runTransitionPush(0);  // ปิด log ซ้ำซ้อนตอนผลัก
    } else {
      // โหมดปิด Auto: หดก้าน แล้วรอคนหยิบออก
      retractPart();
      showActionMessage("Pick up Part!");
      
      // ฟังก์ชันรอหยิบของ (ถ้าไม่มีของวางอยู่แต่แรก มันจะตรวจว่าว่างและเด้งผ่านไปรอบต่อไปทันที)
      if (waitForPartRemoval() == -1) {
        Serial.println("⚠️ Dry Run Aborted during Part Removal!");
        break;
      }
    }
  }

  Serial.println("======= [End] Dry Run Mode =======");
  
  // เมื่อทำงานครบเซ็ต หรือกดยกเลิก ให้รีเฟรชหน้าจอกลับไปเมนูหลัก
  currentMenu = 0;
  cursorIndex = 0;
  scrollOffset = 0;
  updateDisplay();
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

void checkEmergencyReboot() {
  // ถ้ามีการกดปุ่ม Rotary (SW)
  if (digitalRead(SW_PIN) == LOW) {
    unsigned long pressStartTime = millis();
    bool isLongPress = false;

    // วนลูปรอจนกว่าจะปล่อยปุ่ม หรือกดค้างเกิน 3 วินาที
    while (digitalRead(SW_PIN) == LOW) {
      if (millis() - pressStartTime >= 3000) {
        isLongPress = true;
        break;
      }
    }

    // ถ้ากดค้างครบ 3 วินาที สั่ง Reboot ทันที
    if (isLongPress) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_8x13B_tf); // ถ้าใช้ฟอนต์อื่นอยู่ เปลี่ยนชื่อให้ตรงได้เลยครับ
      int textX = (128 - u8g2.getStrWidth("REBOOTING...")) / 2;
      u8g2.drawStr(textX, 35, "REBOOTING...");
      u8g2.sendBuffer();
      
      Serial.println("⚠️ System Reboot Triggered Globally!");
      
      digitalWrite(buzzerPin, HIGH);
      delay(1000); 
      digitalWrite(buzzerPin, LOW);
      
      wdt_enable(WDTO_15MS); 
      while(1); // ล็อคโปรแกรมรอ Watchdog ตัดไฟ
    }
    // ถ้ากดไม่ถึง 3 วินาที ฟังก์ชันจะจบการทำงานและปล่อยโปรแกรมรันต่อปกติ
  }
}
