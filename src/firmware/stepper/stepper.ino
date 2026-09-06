// --- กำหนดขาสัญญาณควบคุม DRV8825 ---
const int stepPin = 10;  // ขา STEP สำหรับส่งพัลส์สั่งหมุน
const int dirPin  = 11;  // ขา DIR สำหรับกำหนดทิศทาง (HIGH/LOW)
const int enablePin = 12; // ขา ENABLE สำหรับเปิด/ปิดการจ่ายไฟเข้าขดลวดมอเตอร์[cite: 2]

const int stepsPerRev = 200; // Nema 8 หมุน 1 รอบใช้ 200 สเต็ป (แบบ Full Step)

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);

  // เปิดใช้งานไดรเวอร์ DRV8825 (สั่ง LOW)
  digitalWrite(enablePin, LOW);
  delay(100);
}

void loop() {
  // --- 1. หมุนตามเข็มนาฬิกา (Forward) ---
  digitalWrite(dirPin, HIGH);
  for (int i = 0; i < stepsPerRev; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1500); // ปรับความเร็วตรงนี้ (ค่ายิ่งมาก ยิ่งหมุนช้าและสมูท)
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1500);
  }

  delay(1000); // พักรอ 1 วินาทีที่จุดปลายทาง

  // --- 2. หมุนทวนเข็มนาฬิกา (Reverse) ---
  digitalWrite(dirPin, LOW);
  for (int i = 0; i < stepsPerRev; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1500);
  }

  delay(1000); // พักรอ 1 วินาทีที่จุดเริ่มต้น
}
