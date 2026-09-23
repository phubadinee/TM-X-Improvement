int read_st188() {
  int st188_val = analogRead(st188Pin);
  // Serial.print("ST188 Value : ");
  // Serial.println(st188_val);
  int st188_val_map = map(st188_val, 0, 1023, 0, 100);
  //  Serial.print("ST188 Value Map : ");
  //  Serial.println(st188_val_map);

  return st188_val_map;
}

int runDetectPart(int showlog) {
  if (showlog == 1) {
    showActionMessage("Detecting Part...");
  }
  Serial.println("======= [Start] Detecting Part =======");
  
  // ให้จอ OLED โหลดข้อความเสร็จ ไม่ต้องรอนานถึง 1 วิ
  delay(100); 
  
  int detect_val = read_st188();

  while (detect_val >= st188Threshold) {

    checkEmergencyReboot();
    
    // หมายเหตุ: ปิดการโชว์คำว่า "No Part..." ในลูปไว้ถือว่าดีแล้วครับ 
    // เพราะถ้าโชว์ตลอด จอจะกระพริบรัวๆ และทำให้ลูปอ่านเซ็นเซอร์ทำงานช้าลง
    
    // --- ตรวจจับคำสั่ง <STOP> ผ่าน Serial ---
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      if (cmd == "<STOP>") {
        Serial.println("⚠️ Detection Aborted via Serial!");
        return -1; // ส่งค่า -1 เพื่อบอกฟังก์ชันหลักให้หยุดทำงาน
      }
    }

    // --- ตรวจจับปุ่มกด STOP/BACK ---
    if (digitalRead(STOP_BTN_PIN) == LOW) {
      delay(50);
      if (digitalRead(STOP_BTN_PIN) == LOW) {
        while (digitalRead(STOP_BTN_PIN) == LOW);
        return -1;
      }
    }

    detect_val = read_st188();
  }

  Serial.println("======= [End] Detecting Part =======");
  Serial.println();
  
  beep(100);
  
  // หน่วงเวลาแค่ 0.3 วินาที ให้ผู้ใช้งานดึงมือออกจากการวางชิ้นงาน 
  // (ถ้าใช้ 1000ms ก้านจะรอนานเกินไปกว่าจะเริ่มดัน)
  delay(300); 

  return 1; // ส่ง 1 กลับไปแปลว่าเจอชิ้นงานปกติ
}

int runDetectPart_TEST() {
  Serial.println("======= [Start] Detecting Part =======");
  
  // ลดเวลาเสียเปล่าในโหมด TEST
  delay(100); 

  for_beep();

  Serial.println("======= [End] Detecting Part ======="); 
  Serial.println();
  
  beep(100);
  delay(300); 
  return 1;
}

int readPositionSmoothly() {
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(feedbackPin);
  }
  return sum / 5;
}

void retractPart() {
  Serial.println("======= [Start] Retracting Part =======");

  // 1. เปิดสัญญาณ PWM ก่อนสั่งมอเตอร์ขยับ (ป้องกันการกระตุกด้วยการส่งค่าเดิมไปก่อน)
  actuator.writeMicroseconds(servoPWM);
  if (!actuator.attached()) actuator.attach(rcPin);

  currentState = FAST_RETRACT;
  bool isRetracting = true;

  while (isRetracting) {
    checkEmergencyReboot();
    
    int currentPos = readPositionSmoothly();

    switch (currentState) {
      // 4. หดกลับ (แบบเร็ว)
      case FAST_RETRACT:
        if (millis() - lastMoveTime >= fastInterval) {
          if (servoPWM > 998) servoPWM -= fastRetractStep;
          actuator.writeMicroseconds(servoPWM);
          lastMoveTime = millis();
        }

        if (currentPos >= POS_RETRACTED) {
          Serial.println("-> Fully retracted: Finishing retract sequence");
          isRetracting = false; // ออกจากฟังก์ชันหดกลับ
        }
        break;
    }

    if (isRetracting) {
      // Serial.print("Retract State: ");
      // Serial.print(currentState);
      // Serial.print(" | Feedback: ");
      // Serial.print(currentPos);
      // Serial.print(" | PWM: ");
      // Serial.println(servoPWM);
    }
    delay(10);
  }

  // =========================================================
  // 2. ตัดสัญญาณ PWM ทันทีที่หดสุด เพื่อป้องกันมอเตอร์ดันค้างและมีเสียงตื้ด
  actuator.detach(); 
  // =========================================================

  Serial.println("======= [End] Part Retracted =======");
  Serial.println();
}

// ----------------------------------------------------

void extendPart(int targetType) {
  Serial.print("======= [Start] Extending Part (Type: ");
  Serial.print(targetType);
  Serial.println(") =======");

  // เปิดสัญญาณ PWM ก่อนสั่งมอเตอร์ยืดออก
  actuator.writeMicroseconds(servoPWM);
  if (!actuator.attached()) actuator.attach(rcPin);

  int targetPos;

  if (targetType == 1) {
    targetPos = POS_EXTEND_SHORT;
  } else if (targetType == 2) {
    targetPos = POS_EXTEND_MID;
  } else if (targetType == 3) {
    targetPos = POS_EXTEND_LONG;
  } else {
    targetPos = POS_EXTEND_SHORT; 
  }

  int slowThreshold = targetPos + ((POS_RETRACTED - targetPos) / 3);
  currentState = FAST_EXTEND;
  bool isExtending = true;

  while (isExtending) {
    checkEmergencyReboot();
    
    int currentPos = readPositionSmoothly();

    switch (currentState) {
      case FAST_EXTEND:
        if (millis() - lastMoveTime >= fastInterval) {
          if (servoPWM < 2000) servoPWM += fastExtendStep;
          actuator.writeMicroseconds(servoPWM);
          lastMoveTime = millis();
        }

        if (currentPos <= slowThreshold) {
          Serial.println("-> Reached slow threshold: Switching to SLOW mode");
          currentState = SLOW_EXTEND;
        }
        break;

      case SLOW_EXTEND:
        if (millis() - lastMoveTime >= slowInterval) {
          if (servoPWM < 2000) servoPWM += slowExtendStep;
          actuator.writeMicroseconds(servoPWM);
          lastMoveTime = millis();
        }

        if (currentPos <= targetPos) {
          Serial.println("-> Reached MAX extension: Pausing");
          pauseStartTime = millis();
          currentState = PAUSE;
        }
        break;

      case PAUSE:
        actuator.writeMicroseconds(servoPWM);

        if (millis() - pauseStartTime >= pauseDuration) {
          Serial.println("-> Pause finished: Ready to retract");
          isExtending = false;
        }
        break;
    }
    delay(10);
  }

  Serial.println("======= [End] Part Extended =======");
  Serial.println();
}

// ----------------------------------------------------

int extendPart_check(int targetType) {
  Serial.print("======= [Start] Extending Part (Type: ");
  Serial.print(targetType);
  Serial.println(") =======");

  // เปิดสัญญาณ PWM ก่อนสั่งมอเตอร์ยืดออก
  actuator.writeMicroseconds(servoPWM);
  if (!actuator.attached()) actuator.attach(rcPin);

  int targetPos;

  if (targetType == 1) {
    targetPos = POS_EXTEND_SHORT;
  } else if (targetType == 2) {
    targetPos = POS_EXTEND_MID;
  } else if (targetType == 3) {
    targetPos = POS_EXTEND_LONG;
  } else {
    targetPos = POS_EXTEND_SHORT; 
  }

  int slowThreshold = targetPos + ((POS_RETRACTED - targetPos) / 3);
  int tolerance = 20; 
  unsigned long startExtendTime = millis();
  const unsigned long maxExtendTime = 4000; 

  currentState = FAST_EXTEND;
  bool isExtending = true;

  while (isExtending) {
    checkEmergencyReboot();
    
    int currentPos = readPositionSmoothly();

    if (millis() - startExtendTime > maxExtendTime) {
      if (currentPos <= targetPos + tolerance) {
        Serial.println("-> Reached MAX extension (Timeout triggered, but within tolerance)");
        pauseStartTime = millis();
        currentState = PAUSE;
        startExtendTime = millis(); 
      } else {
        Serial.println("⚠️ ERROR: Actuator stalled! (Did not reach target)");
        return -1; 
      }
    }

    switch (currentState) {
      case FAST_EXTEND:
        if (millis() - lastMoveTime >= fastInterval) {
          if (servoPWM < 2000) servoPWM += fastExtendStep;
          actuator.writeMicroseconds(servoPWM);
          lastMoveTime = millis();
        }

        if (currentPos <= slowThreshold) {
          Serial.println("-> Reached slow threshold: Switching to SLOW mode");
          currentState = SLOW_EXTEND;
        }
        break;

      case SLOW_EXTEND:
        if (millis() - lastMoveTime >= slowInterval) {
          if (servoPWM < 2000) servoPWM += slowExtendStep;
          actuator.writeMicroseconds(servoPWM);
          lastMoveTime = millis();
        }

        if (currentPos <= targetPos + tolerance) {
          Serial.println("-> Reached MAX extension: Pausing");
          pauseStartTime = millis();
          currentState = PAUSE;
        }
        break;

      case PAUSE:
        actuator.writeMicroseconds(servoPWM);

        if (millis() - pauseStartTime >= pauseDuration) {
          Serial.println("-> Pause finished: Ready to retract");
          isExtending = false;
        }
        break;
    }
    delay(10);
  }

  Serial.println("======= [End] Part Extended =======");
  Serial.println();
  return 1; 
}


void runAlignPart() {
  showActionMessage("Aligning Part...");

  extendPart(1);   // สั่งยืดออก และรอ 5 วินาที
  retractPart();  // สั่งหดกลับ

  Serial.println("======= Cycle Completed =======");
}
