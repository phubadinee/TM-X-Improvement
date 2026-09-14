void runAlignment() {

  while (1) {

    int currentPos = readPositionSmoothly();

    // จัดการการทำงานตามสถานะปัจจุบัน
    switch (currentState) {

      // 1. ยืดออกช่วงแรก (แบบเร็ว)
      case FAST_EXTEND:
        if (millis() - lastMoveTime >= fastInterval) {
          if (servoPWM < 2000) servoPWM += fastExtendStep;
          actuator.writeMicroseconds(servoPWM);
          lastMoveTime = millis();
        }

        // เมื่อถึงค่าที่กำหนด ให้เปลี่ยนเป็นโหมดช้า
        if (currentPos <= POS_SLOW_START) {
          Serial.println("-> Reached slow threshold: Switching to SLOW mode");
          currentState = SLOW_EXTEND;
        }
        break;

      // 2. ยืดออกช่วงท้าย (แบบช้า)
      case POS_SLOW_START:
      case SLOW_EXTEND:
        if (millis() - lastMoveTime >= slowInterval) {
          if (servoPWM < 2000) servoPWM += slowExtendStep;
          actuator.writeMicroseconds(servoPWM);
          lastMoveTime = millis();
        }

        // เมื่อถึงระยะยืดสุด ให้หยุดและเริ่มจับเวลาพัก 5 วินาที
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
          Serial.println("-> Pause finished: Retracting fast");
          currentState = FAST_RETRACT;
        }
        break;

      // 4. หดกลับ (แบบเร็ว)
      case FAST_RETRACT:
        if (millis() - lastMoveTime >= fastInterval) {
          if (servoPWM > 998) servoPWM -= fastRetractStep;
          actuator.writeMicroseconds(servoPWM);
          lastMoveTime = millis();
        }

        // เมื่อหดกลับมาถึงตำแหน่งเริ่มต้น ให้วนลูปใหม่
        if (currentPos >= POS_RETRACTED) {
          Serial.println("-> Fully retracted: Restarting cycle");
          break;
        }
        break;
    }

    // แสดงผลเช็คสถานะผ่าน Serial Monitor
    Serial.print("State: ");
    Serial.print(currentState);
    Serial.print(" | Feedback: ");
    Serial.print(currentPos);
    Serial.print(" | PWM: ");
    Serial.println(servoPWM);

    delay(10);


  }



}
