#include <AccelStepper.h>

const int stepPin = 35; 
const int dirPin = 34;
const int limitSwitchPin = 31; 
const int enPin = 42; // เพิ่มขา Enable สำหรับ DRV8825

AccelStepper stepper(1, stepPin, dirPin);

int dis = 800;
long positions[4] = {0, 885, 1650, 2400};
int currentPosIndex = 0;
int state = 0; 

void setup() {
  Serial.begin(115200);
  
  pinMode(limitSwitchPin, INPUT_PULLUP);
  
  // ตั้งค่าและเปิดใช้งาน DRV8825
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW); // LOW = เปิดให้ไดร์เวอร์ทำงาน

  stepper.setMaxSpeed(800);      // ยิ่งค่านี้น้อย แรงบิดจะยิ่งเยอะ
  stepper.setAcceleration(400); 
  
  Serial.println("Start Homing...");
}

void loop() {
  switch (state) {
    
    case 0:
    case 2:
      if (digitalRead(limitSwitchPin) == LOW) {
        stepper.stop();                
        
        // --- เพิ่มโค้ด 3 บรรทัดนี้เพื่อถอยออกจากสวิตช์ ---
        stepper.setCurrentPosition(0); // 1. เซ็ตตำแหน่งที่ชนเป็น 0 ชั่วคราว
        stepper.runToNewPosition(50);  // 2. สั่งหมุนไปทางขวา 50 สเต็ป (ปรับเพิ่ม/ลดตัวเลข 50 ได้ตามระยะที่ต้องการ)
        stepper.setCurrentPosition(0); // 3. เซ็ตตำแหน่งที่ถอยออกมาเป็นตำแหน่ง Home (0) ของจริง
        // -------------------------------------------
        
        Serial.println("Limit Switch Pressed! Reached Home.");
        
        // ตัดไฟมอเตอร์ตอนหยุดพัก
        digitalWrite(enPin, HIGH); // HIGH = ตัดไฟเข้ามอเตอร์
        delay(1000); 
        digitalWrite(enPin, LOW);  // กลับมาจ่ายไฟเตรียมตัววิ่งต่อ
        
        state = 1;
        currentPosIndex = 0;
        stepper.moveTo(positions[currentPosIndex]); 
        
      } else {
        stepper.setSpeed(-500); 
        stepper.runSpeed();     
      }
      break;

    case 1:
      if (stepper.distanceToGo() == 0) {
        Serial.print("Reached Position: ");
        Serial.println(currentPosIndex + 1);
        
        // ตัดไฟมอเตอร์ตอนหยุดพักในแต่ละตำแหน่ง
        digitalWrite(enPin, HIGH); 
        delay(1000); 
        digitalWrite(enPin, LOW); 
        
        currentPosIndex++; 
        
        if (currentPosIndex < 4) {
          stepper.moveTo(positions[currentPosIndex]);
        } else {
          Serial.println("Completed 4 positions. Returning back...");
          state = 2; 
        }
      }
      
      stepper.run(); 
      break;
  }
}
