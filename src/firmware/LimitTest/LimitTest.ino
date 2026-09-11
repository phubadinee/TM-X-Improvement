#include <AccelStepper.h>

// กำหนดขา Stepper (ใช้ขาที่สามารถเป็น OUTPUT ได้จริงๆ)
const int stepPin = 35;
const int dirPin = 34;
// กำหนดขาสำหรับ Limit Switch
const int limitSwitchPin = 31;

AccelStepper stepper(1, stepPin, dirPin);

// กำหนดระยะทั้ง 4 ตำแหน่ง (หน่วยเป็น Step นับจากจุดที่ชนลิมิตสวิตซ์)
// *สามารถปรับตัวเลขเหล่านี้ให้เข้ากับระยะรางจริงได้*
long positions[4] = {0, 780, 750*2, 750*3};
int currentPosIndex = 0;

// ตัวแปรสถานะการทำงาน (State)
// 0 = Homing (วิ่งหาลิมิตสวิตซ์)
// 1 = กำลังเคลื่อนที่ไปตาม 4 ตำแหน่ง
// 2 = ครบ 4 ตำแหน่งแล้ว ถอยกลับมาหาลิมิตสวิตซ์
int state = 0;

void setup() {
  Serial.begin(115200);
  pinMode(limitSwitchPin, INPUT);

  // บังคับขา Stepper ให้เป็น OUTPUT และส่งสัญญาณ LOW เพื่อปิดการทำงานชั่วคราว
  pinMode(34, OUTPUT);
  pinMode(35, OUTPUT);
  //  digitalWrite(34, LOW);
  //  digitalWrite(35, LOW);

  stepper.setMaxSpeed(2000); 
  stepper.setAcceleration(1000);

  Serial.println("Start Homing...");
}

void loop() {
  //  int x = digitalRead(limitSwitchPin);
  //  Serial.println(x);
  switch (state) {

    // สถานะ 0 และ 2: การวิ่งกลับไปชน Limit Switch (Homing / Return)
    case 0:
    case 2:
      // เช็คว่าลิมิตสวิตซ์ถูกกดหรือยัง (LOW = ถูกกด)
      if (digitalRead(limitSwitchPin) == LOW) {
        stepper.setCurrentPosition(0); // เซ็ตตำแหน่งนี้ให้เป็นจุดเริ่มต้น (0)
        stepper.stop();                // สั่งหยุดมอเตอร์

        Serial.println("Limit Switch Pressed! Reached Home.");
        delay(1000); // หน่วงเวลา 1 วินาทีก่อนเริ่มสเตปถัดไป

        // เมื่อชนสวิตซ์แล้ว ให้เริ่มทำงานเลื่อนไป 4 ตำแหน่ง
        state = 1;
        currentPosIndex = 0;
        stepper.moveTo(positions[currentPosIndex]); // สั่งให้เตรียมวิ่งไปตำแหน่งที่ 1

      } else {
        // ถ้ายะงไม่ชนสวิตซ์ ให้มอเตอร์หมุนถอยหลังด้วยความเร็วคงที่
        stepper.setSpeed(-500); // ค่าติดลบ = หมุนกลับ
        stepper.runSpeed();     // ใช้ runSpeed() เพราะต้องการให้วิ่งด้วยความเร็วคงที่
      }
      break;

    // สถานะ 1: เลื่อนไป 4 ตำแหน่ง
    case 1:
      // เช็คว่ามอเตอร์วิ่งถึงตำแหน่งที่สั่งไว้ (distanceToGo = 0)
      if (stepper.distanceToGo() == 0) {
        Serial.print("Reached Position: ");
        Serial.println(currentPosIndex + 1);

        delay(1000); // หยุดพักทำงานที่ตำแหน่งนี้ 1 วินาที (ปรับหรือเอาออกได้ตามระบบงานจริง)

        currentPosIndex++; // เพิ่มค่า Index เพื่อเตรียมไปตำแหน่งถัดไป

        if (currentPosIndex < 4) {
          // ถ้ายังไม่ครบ 4 ตำแหน่ง ให้สั่งวิ่งไปตำแหน่งถัดไป
          stepper.moveTo(positions[currentPosIndex]);
        } else {
          // ถ้าครบ 4 ตำแหน่งแล้ว ให้เปลี่ยนสถานะถอยกลับ
          Serial.println("Completed 4 positions. Returning back...");
          state = 2;
        }
      }

      stepper.run(); // ใช้ run() คู่กับ moveTo() เพื่อให้มอเตอร์มีอัตราเร่ง (Acceleration)
      break;
  }
}
