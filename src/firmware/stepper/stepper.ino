#include <AccelStepper.h>

// กำหนดค่าโหมด Half-step (8 พินซีเควนซ์) สำหรับ 28BYJ-48
#define HALFSTEP 8

// กำหนดขาที่ต่อกับบอร์ด ULN2003 (เรียงตามลำดับ 1, 3, 2, 4 เพื่อความถูกต้องของขดลวด)
#define motorPin1  10  // IN1
#define motorPin2  11  // IN2
#define motorPin3  12 // IN3
#define motorPin4  13  // IN4

// สร้างออบเจ็กต์ AccelStepper ในโหมด Half-step
AccelStepper stepper(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

void setup() {
  Serial.begin(115200);

  // ตั้งค่าความเร็วสูงสุดและความเร่งเพื่อให้มอเตอร์ไต่รอบไปถึงจุดสูงสุดได้ไวที่สุด
  // หมายเหตุ: สำหรับ 28BYJ-48 ความเร็วสูงสุดที่ใช้งานได้จริงมักจะอยู่ราวๆ 1000-1200 Steps/sec 
  stepper.setMaxSpeed(1000.0);    
  stepper.setAcceleration(800.0); 
  
  // สั่งให้หมุนไปข้างหน้าแบบต่อเนื่องยาวๆ
  stepper.moveTo(2048000); 

  Serial.println("=== เริ่มต้นควบคุม 28BYJ-48 ด้วยโหมด Half-step สปีดสูงสุด ===");
}

void loop() {
  // ฟังก์ชันนี้ต้องถูกเรียกใช้งานตลอดเวลาเพื่อประมวลผลพัลส์การหมุนของมอเตอร์
  stepper.run();
}