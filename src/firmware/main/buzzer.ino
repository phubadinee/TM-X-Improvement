void beep(int delay_) {
  if (isMuted) return; // ถ้าเปิด Mute ให้ข้ามทันที
  digitalWrite(buzzerPin, 1);
  delay(delay_);
  digitalWrite(buzzerPin, 0);
  delay(delay_);
}

void beepHigh() {
  if (isMuted) return;
  digitalWrite(buzzerPin, 1);
}

void beepLow() {
  if (isMuted) return;
  digitalWrite(buzzerPin, 0);
}

void long_beep() {
  if (isMuted) {
    delay(2000); // ชดเชยเวลาดีเลย์ เพื่อไม่ให้ลอจิกเวลาเพี้ยนเวลาปิดเสียง
    return;
  }
  digitalWrite(buzzerPin, 1);
  delay(1000);
  digitalWrite(buzzerPin, 0);
  delay(1000);
}

void for_beep() {
  if (isMuted) return;
  beep(100); beep(100); beep(100);
}

void load_beep() {
  if (isMuted) return;
  for (int i = 0; i < 5; i++) {
    beep(100);
    delay(500);
  }
}

void for_beep_fast() {
  if (isMuted) return;
  beep(50); beep(50); beep(50);
}
