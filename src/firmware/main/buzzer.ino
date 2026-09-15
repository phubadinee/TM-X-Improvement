void beep(int delay_) {
  digitalWrite(buzzerPin, 1);
  delay(delay_);
  digitalWrite(buzzerPin, 0);
  delay(delay_);
}

void beepHigh() {
  digitalWrite(buzzerPin, 1);
}

void beepLow() {
  digitalWrite(buzzerPin, 0);
}

void long_beep() {
  digitalWrite(buzzerPin, 1);
  delay(1000);
  digitalWrite(buzzerPin, 0);
  delay(1000);
}

void for_beep() {
  beep(100); beep(100); beep(100);
}

void load_beep() {
  for (int i = 0; i < 5; i++) {
    beep(100);
    delay(500);
  }
}

void for_beep_fast() {
  beep(50); beep(50); beep(50);
}
