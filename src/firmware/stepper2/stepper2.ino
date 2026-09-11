#include <AccelStepper.h>

const int stepPin = 35; 
const int dirPin = 34;
const int limitSwitchPin = 31; 
const int enPin = 42; 

AccelStepper stepper(1, stepPin, dirPin);

long positions[4] = {0, 885, 1650, 2400};

void setup() {
  Serial.begin(115200);
  
  pinMode(limitSwitchPin, INPUT_PULLUP);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW); // จ่ายไฟให้ DRV8825

  stepper.setMaxSpeed(800);      
  stepper.setAcceleration(400); 
}

void loop() {
  // 1. กระบวนการ Homing
  Serial.println("Start Homing...");
  digitalWrite(enPin, LOW); // จ่ายไฟมอเตอร์
  
  // วิ่งซ้ายหา Limit Switch ด้วยความเร็วคงที่
  stepper.setSpeed(-500);   // ค่าติดลบ = วิ่งซ้าย
  while (digitalRead(limitSwitchPin) == HIGH) {
    stepper.runSpeed();     
  }
  
  stepper.stop();
  
  // --- ขั้นตอนที่เพิ่ม: ชนแล้วถอยซ้ายไปอีกหน่อย (เช่น ถอยไป -50 สเต็ปจากจุดชน) ---
  long targetOffset = stepper.currentPosition() - 0; // ถอยหลังซ้ายเพิ่มไปอีก 50 สเต็ป
  stepper.setSpeed(-400); // ใช้ความเร็วติดลบเพื่อถอยไปทางซ้าย (ได้แรงบิดเต็มที่)
  while (stepper.currentPosition() > targetOffset) {
    stepper.runSpeed();
  }
  stepper.stop();
  // --------------------------------------------------------------------------

  // เซ็ตตำแหน่งตรงจุดที่ถอยเสร็จนี้ให้เป็นตำแหน่ง 0 ใหม่
  stepper.setCurrentPosition(0); 
  
  Serial.println("Homing Complete!");
  pauseMotor(); // หยุดพัก 1 วินาที

  // 2. วิ่งไปตามตำแหน่งทั้ง 4 ที่ตั้งไว้
  for (int i = 0; i < 4; i++) {
    Serial.print("Going to Position: ");
    Serial.println(i + 1);
    
    // วิ่งไปที่ตำแหน่งและรอจนกว่าจะถึง
    stepper.runToNewPosition(positions[i]); 
    
    // พอถึงแล้ว ตัดไฟพัก 1 วินาที
    pauseMotor(); 
  }
  
  Serial.println("Completed 4 positions. Returning to Home...");
}

// ฟังก์ชันเสริมสำหรับหยุดพักและตัดไฟ (เขียนแยกเพื่อให้โค้ดอ่านง่าย)
void pauseMotor() {
  digitalWrite(enPin, HIGH); // ตัดไฟลดความร้อน
  delay(1000); 
  digitalWrite(enPin, LOW);  // จ่ายไฟกลับเตรียมทำงานต่อ
}
