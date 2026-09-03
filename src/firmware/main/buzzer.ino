void beep(){
  digitalWrite(buzzerPin, 1);
  delay(100);
  digitalWrite(buzzerPin, 0);
  delay(100);
}

void long_beep(){
  digitalWrite(buzzerPin, 1);
  delay(1000);
  digitalWrite(buzzerPin, 0);
  delay(1000);
}

void for_beep(){
  beep();beep();beep();
}