int read_st188() {
  int st188_val = analogRead(st188Pin);
  // Serial.print("ST188 Value : ");
  // Serial.println(st188_val);
  int st188_val_map = map(st188_val, 0, 1023, 0, 100);
  Serial.print("ST188 Value Map : ");
  Serial.println(st188_val_map);

  return st188_val_map;
}

int runDetectPart() {
  showActionMessage("Detecting Part...");
  Serial.println("======= [Start] Detecting Part =======");
  delay(1000);
  int detect_val = read_st188();

  while (detect_val >= 80) {
    showActionMessage("No Part...");
    detect_val = read_st188();
    //    Serial.println(detect_val);
  }
  showActionMessage("Part Detected !!!");
  Serial.println("======= [End] Detecting Part ======="); Serial.println();
  beep(100);
  delay(1000);

  // return detect_status;
}

int runDetectPart_TEST() {
  showActionMessage("Detecting Part...");
  Serial.println("======= [Start] Detecting Part =======");
  delay(1000);

  for_beep();
  
  showActionMessage("Part Detected !!!");
  Serial.println("======= [End] Detecting Part ======="); Serial.println();
  beep(100);
  delay(1000);

}


int readPositionSmoothly() {
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(feedbackPin);
  }
  return sum / 5;
}

// =======================================================
// ฟังก์ชันสำหรับ ยืดออก (Fast -> Slow -> Pause 5s)
// =======================================================
void extendPart() {
  Serial.println("======= [Start] Extending Part =======");
  
  currentState = FAST_EXTEND; 
  bool isExtending = true; 

  while (isExtending) { 
    int currentPos = readPositionSmoothly();

    switch (currentState) {
      // 1. ยืดออกช่วงแรก (แบบเร็ว)
      case FAST_EXTEND:
        if (millis() - lastMoveTime >= fastInterval) {
          if (servoPWM < 2000) servoPWM += fastExtendStep;
          actuator.writeMicroseconds(servoPWM);
          lastMoveTime = millis();
        }

        if (currentPos <= POS_SLOW_START) {
          Serial.println("-> Reached slow threshold: Switching to SLOW mode");
          currentState = SLOW_EXTEND;
        }
        break;

      // 2. ยืดออกช่วงท้าย (แบบช้า)
      case SLOW_EXTEND:
        if (millis() - lastMoveTime >= slowInterval) {
          if (servoPWM < 2000) servoPWM += slowExtendStep;
          actuator.writeMicroseconds(servoPWM);
          lastMoveTime = millis();
        }

        if (currentPos <= POS_EXTENDED) {
          Serial.println("-> Reached MAX extension: Pausing for 5 seconds");
          pauseStartTime = millis();
          currentState = PAUSE;
        }
        break;

      // 3. หยุดรอ 5 วินาทีที่ตำแหน่งยืดสุด
      case PAUSE:
        actuator.writeMicroseconds(servoPWM); // ล็อคตำแหน่งไว้

        if (millis() - pauseStartTime >= pauseDuration) {
          Serial.println("-> Pause finished: Ready to retract");
          isExtending = false; // ออกจากฟังก์ชันยืดออก
        }
        break;
    }

    if (isExtending) {
      Serial.print("Extend State: ");
      Serial.print(currentState);
      Serial.print(" | Feedback: ");
      Serial.print(currentPos);
      Serial.print(" | PWM: ");
      Serial.println(servoPWM);
    }
    delay(10);
  }
  
  Serial.println("======= [End] Part Extended ======="); 
  Serial.println();
}


void retractPart() {
  Serial.println("======= [Start] Retracting Part =======");
  
  currentState = FAST_RETRACT; 
  bool isRetracting = true; 

  while (isRetracting) { 
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
      Serial.print("Retract State: ");
      Serial.print(currentState);
      Serial.print(" | Feedback: ");
      Serial.print(currentPos);
      Serial.print(" | PWM: ");
      Serial.println(servoPWM);
    }
    delay(10);
  }
  
  Serial.println("======= [End] Part Retracted ======="); 
  Serial.println();
}


void runAlignPart() {
  showActionMessage("Aligning Part...");
  
  extendPart();   // สั่งยืดออก และรอ 5 วินาที
  retractPart();  // สั่งหดกลับ
  
  Serial.println("======= Cycle Completed =======");
}
