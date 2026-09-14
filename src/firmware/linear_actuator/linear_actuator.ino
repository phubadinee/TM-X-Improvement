#include <Servo.h>

// ==========================================
// 1. การตั้งค่าขา Pin (Pin Configuration)
// ==========================================
const int feedbackPin = A4;  // สาย Feedback ต่อเข้า A4
const int rcPin = 44;        // สาย RC (PWM) ต่อเข้า Pin 44

Servo actuator;

// ==========================================
// 2. กำหนดค่าตัวแปรขอบเขต (Thresholds & Limits)
// ==========================================
const int POS_RETRACTED = 600;  // ค่า Feedback เมื่อหดกลับสุด (สำหรับเริ่มวนลูปใหม่)
const int POS_SLOW_START = 300; // ค่า Feedback ที่เริ่มเปลี่ยนเป็นความเร็วช้า
const int POS_EXTENDED = 100;   // ค่า Feedback เป้าหมายเมื่อยืดสุด

// ==========================================
// 3. กำหนดค่าความเร็วและระยะเวลา (Speed & Timing)
// ==========================================
const int fastInterval = 1;     // ความเร็วช่วงขยับไว (มิลลิวินาทีต่อสเตป)
const int slowInterval = 5;    // ความเร็วช่วงขยับช้า (มิลลิวินาทีต่อสเตป)

const int fastExtendStep = 10;   // แต้ม PWM ที่บวกเพิ่มตอนยืดเร็ว
const int slowExtendStep = 1;   // แต้ม PWM ที่บวกเพิ่มตอนยืดช้า (นุ่มนวล)
const int fastRetractStep = 3;  // แต้ม PWM ที่ลดลงตอนหดกลับเร็ว

const unsigned long pauseDuration = 1000; // เวลาหยุดพักเมื่อยืดสุด (5 วินาที)

// ตัวแปรระบบภายใน
int servoPWM = 998;
unsigned long lastMoveTime = 0;
unsigned long pauseStartTime = 0;

// กำหนดสถานะการทำงาน (State Machine)
enum State { FAST_EXTEND, SLOW_EXTEND, PAUSE, FAST_RETRACT };
State currentState = FAST_EXTEND;

// ฟังก์ชันกรองสัญญาณรบกวน Analog
int readPositionSmoothly() {
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(feedbackPin);
  }
  return sum / 5;
}

void setup() {
  Serial.begin(115200);
  actuator.attach(rcPin);

  // เซ็ตตำแหน่งเริ่มต้นที่จุดหดสุด
  servoPWM = 998;
  actuator.writeMicroseconds(servoPWM);
  delay(500);

  Serial.println("System Ready: Auto Sequence Started...");
}

void loop() {
  runAlignment();
  while (1) {
    Serial.println("x");
  }
}
