void runTransitionPush(int showlog) {

  if (showlog == 1) {
    showActionMessage("Transition Push...");
  }
  Serial.println("======= [Start] Transition Push =======");

  runTransitionRetract(0);

  while (digitalRead(limit_servo) == 1) {
    myServo.write(servo_forward_fast);
  }

  myServo.write(servo_backward);
  delay(3000);

  myServo.write(servo_stop);
  Serial.println("======= [End] Transition Push ======="); Serial.println();

}


void runTransitionRetract(int showlog) {

  if (showlog == 1) {
    showActionMessage("Transition Retract...");
  }
  Serial.println("======= [Start] Transition Retract =======");

  myServo.write(servo_backward);
  delay(500);

  myServo.write(servo_stop);
  Serial.println("======= [End] Transition Push ======="); Serial.println();

}
