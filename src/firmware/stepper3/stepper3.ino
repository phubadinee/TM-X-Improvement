#include <AccelStepper.h>

const int stepPin = 35; 
const int dirPin = 34;
const int limitSwitchPin = 31; 
const int enPin = 42; 

AccelStepper stepper(1, stepPin, dirPin);

long positions[4] = {0, 885, 1650, 2400}; // ตำแหน่งช่องที่ 1, 2, 3, 4
int slotCounts[4] = {0, 0, 0, 0};          // ตัวนับจำนวนของในแต่ละช่อง (สูงสุด 4 ชิ้น)

void setup() {
  Serial.begin(115200);
  
  pinMode(limitSwitchPin, INPUT_PULLUP);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);

  stepper.setMaxSpeed(800);      
  stepper.setAcceleration(400); 

  // ทำ Homing ครั้งแรกตอนเปิดเครื่อง
  runHoming();
  
  Serial.println("System Ready.");
  Serial.println("Send '1' -> Go to Slot 1 & 2");
  Serial.println("Send '0' -> Go to Slot 3 & 4");
}

void loop() {
  pauseMotor();
  // ตรวจสอบข้อมูลที่รับเข้ามาทาง Serial
  if (Serial.available() > 0) {
    char incomingChar = Serial.read();
    
    // เรียกใช้งานฟังก์ชันรับค่าและสั่งงานมอเตอร์
    processSlotCommand(incomingChar);
  }
}

// ==========================================
// ฟังก์ชันสำหรับจัดการคำสั่งจาก Serial (0 หรือ 1)
// ==========================================
void processSlotCommand(char command) {
  int targetIndex = -1;
  
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
}

// ฟังก์ชันสำหรับทำ Homing (วิ่งชนซ้าย ถอยออก 50 สเต็ป เซ็ตศูนย์)
void runHoming() {
  Serial.println("Start Homing...");
  digitalWrite(enPin, LOW);
  
  stepper.setSpeed(-500); 
  while (digitalRead(limitSwitchPin) == HIGH) {
    stepper.runSpeed();     
  }
  stepper.stop();
  
  // ถอยซ้ายไปอีก 50 สเต็ปด้วยความเร็วคงที่
  long targetOffset = stepper.currentPosition() - 0;
  stepper.setSpeed(-400); 
  while (stepper.currentPosition() > targetOffset) {
    stepper.runSpeed();
  }
  stepper.stop();
  
  stepper.setCurrentPosition(0);
  Serial.println("Homing Complete!");
  pauseMotor();
}

// ฟังก์ชันสำหรับหน่วงเวลาและตัดไฟมอเตอร์
void pauseMotor() {
  delay(1000);             // รอให้มอเตอร์หยุดนิ่งสนิท 1 วินาที
  digitalWrite(enPin, LOW); // สั่งตัดไฟ (Disable) ค้างไว้ เพื่อลดการกินกระแส
  // ไม่ต้องสั่งเปิดไฟกลับตรงนี้ เพราะตอนจะวิ่งฟังก์ชันหลักจะเปิดให้อัตโนมัติ
}
