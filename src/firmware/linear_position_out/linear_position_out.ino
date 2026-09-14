// ==========================================
// กำหนดขาที่ต่อกับสาย Feedback ของ Actuonix
// ==========================================
const int feedbackPin = A3; 

void setup() {
  // เปิดใช้งาน Serial Monitor ที่ความเร็ว 115200
  Serial.begin(115200);
  Serial.println("Actuonix Position Reader Started...");
}

void loop() {
  // อ่านค่าสัญญาณ Analog จากสาย Feedback
  // ค่าที่ได้จะอยู่ในช่วง 0 ถึง 1023
  int positionValue = analogRead(feedbackPin);

  // พิมพ์แสดงผลทาง Serial Monitor
  Serial.print("Current Position (0-1023): ");
  Serial.println(positionValue);

  // หน่วงเวลา 100 มิลลิวินาที เพื่อไม่ให้ข้อความวิ่งเร็วเกินไป
  delay(100);
}
