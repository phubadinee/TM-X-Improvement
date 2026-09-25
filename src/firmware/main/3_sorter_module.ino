void runSortExecute(char result) {
//  showActionMessage("Sort Execute...");
  Serial.println("======= [Start] Sort Execute =======");
  processSlotCommand(result);
  Serial.println("======= [End] Sort Execute ======="); Serial.println();

}

void processSlotCommand(char command) {
  int targetIndex = -1;

  if (command != 't') {
    // ถ้าได้รับ '1' (เลือกช่อง 1 หรือ 2)
    if (command == '1') {
      if (slotCounts[0] < 4) {
        targetIndex = 0; // ช่องที่ 1
        slotCounts[0]++;
      } else if (slotCounts[1] < 4) {
        targetIndex = 1; // ช่องที่ 2
        slotCounts[1]++;
      } else {
        Serial.println("⚠️ Warning: Slot 1 and 2 are FULL!");
      }
    }
    // ถ้าได้รับ '0' (เลือกช่อง 3 หรือ 4)
    else if (command == '0') {
      if (slotCounts[2] < 4) {
        targetIndex = 2; // ช่องที่ 3
        slotCounts[2]++;
      } else if (slotCounts[3] < 4) {
        targetIndex = 3; // ช่องที่ 4
        slotCounts[3]++;
      } else {
        Serial.println("⚠️ Warning: Slot 3 and 4 are FULL!");
      }
    }

    // ถ้าเลือกตำแหน่งได้ ให้สั่งมอเตอร์วิ่งไป
    if (targetIndex != -1) {
      Serial.print("Signal '");
      Serial.print(command);
      Serial.print("' -> Moving to Slot: ");
      Serial.print(targetIndex + 1);
      Serial.print(" (Items in this slot: ");
      Serial.print(slotCounts[targetIndex]);
      Serial.println("/4)");

      digitalWrite(enPin, LOW); // จ่ายไฟเตรียมวิ่ง
      stepper.runToNewPosition(positions[targetIndex]);

      // ถึงตำแหน่งแล้ว ตัดไฟพัก 1 วินาที
      pauseMotor();
    }
  } else {

    digitalWrite(enPin, LOW); // จ่ายไฟเตรียมวิ่ง
    stepper.runToNewPosition(positions[0]);
    for_beep_fast();
    stepper.runToNewPosition(positions[1]);
    for_beep_fast();
    stepper.runToNewPosition(positions[2]);
    for_beep_fast();
    stepper.runToNewPosition(positions[3]);
    // ถึงตำแหน่งแล้ว ตัดไฟพัก 1 วินาที

    runHoming();
    long_beep();
    pauseMotor();
  }
}


void runHoming() {
  Serial.println("Start Homing...");

  // ให้แน่ใจว่าเปิดไฟเลี้ยงมอเตอร์ก่อน Homing
  digitalWrite(enPin, LOW);

  // 1. ตั้งความเร็วให้วิ่งไปหาลิมิตสวิตช์
  stepper.setSpeed(-400);

  // 2. วนลูปสั่งมอเตอร์หมุน จนกว่าจะชนสวิตช์ (สถานะเป็น LOW)
  while (digitalRead(limit_sorter) == HIGH) {
    stepper.runSpeed();
  }

  // 3. เมื่อชนสวิตช์ (หลุดลูป) ให้ "บังคับหยุดแบบเด็ดขาด"
  stepper.setSpeed(0);             // รีเซ็ตความเร็ว
  stepper.setCurrentPosition(0);   // รีเซ็ตตำแหน่งปัจจุบันเป็น 0
  stepper.moveTo(0);               // เคลียร์เป้าหมายการวิ่ง (Target) ให้เป็น 0 ด้วย (สำคัญมาก!)

  Serial.println("Homing Complete! Motor is fully stopped.");

  // 4. ตัดไฟมอเตอร์เพื่อป้องกันมอเตอร์ล็อคค้างและร้อน (แกนจะหมุนอิสระได้)
  pauseMotor();
}


// ฟังก์ชันสำหรับหน่วงเวลาและตัดไฟมอเตอร์
void pauseMotor() {
  delay(500);            // รอให้มอเตอร์หยุดนิ่งสนิท 1 วินาที
  digitalWrite(enPin, HIGH); // สั่งตัดไฟ (Disable) ค้างไว้ เพื่อลดการกินกระแส
  // ไม่ต้องสั่งเปิดไฟกลับตรงนี้ เพราะตอนจะวิ่งฟังก์ชันหลักจะเปิดให้อัตโนมัติ
}
