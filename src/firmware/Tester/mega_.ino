const int startButtonPin = 2; // กำหนดขาปุ่มกด
int buttonState = 0;
int lastButtonState = 0;

void setup() {
  // ตั้งค่า Serial ให้ตรงกับฝั่ง Pi
  Serial.begin(115200);
  pinMode(startButtonPin, INPUT_PULLUP);
}

void loop() {
  buttonState = digitalRead(startButtonPin);

  // ตรวจจับการกดปุ่ม (Active Low)
  if (buttonState == LOW && lastButtonState == HIGH) {
    // ส่งคำสั่ง Start ไปหา Pi โดยครอบด้วย < >[cite: 5]
    Serial.println("<START>");
    delay(200); // Debounce
  }
  lastButtonState = buttonState;

  // ตรวจสอบข้อมูลที่ Pi ตอบกลับมา
  if (Serial.available() > 0) {
    String incomingMsg = Serial.readStringUntil('\n');
    incomingMsg.trim(); // ตัดเว้นวรรคและ \r ออก

    if (incomingMsg == "<0>") {
      // Logic เมื่อได้รับค่า NG
      // digitalWrite(sorter_pin, HIGH); เป็นต้น
    } 
    else if (incomingMsg == "<1>") {
      // Logic เมื่อได้รับค่า OK
    }
  }
}