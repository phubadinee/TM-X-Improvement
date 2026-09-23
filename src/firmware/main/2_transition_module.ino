void runTransitionPush(int showlog) {
  if (showlog == 1) {
    showActionMessage("Transition Push...");
  }
  Serial.println("======= [Start] Transition Push =======");

  // สั่งดึงกลับเพื่อเช็คความชัวร์ก่อนดัน (อันนี้ใช้เวลา 500ms ตามโค้ดด้านล่าง ถือว่าปลอดภัยแล้ว)
  runTransitionRetract(0);

  // เดินหน้าจนกว่าจะชน Limit Switch
  while (digitalRead(limit_servo) == 1) {
    myServo.write(servo_forward_fast);
  }

  // ถอยกลับ (ลดเวลาจาก 3000ms ให้เหลือแค่พอที่ก้านจะถอยกลับสุด)
  // *คำแนะนำ: ลองปรับตัวเลข 800 ให้น้อยลงได้อีก ถ้ากลไกถอยกลับมาสุดเร็วกว่านี้*
  myServo.write(servo_backward);
  delay(1100); 

  myServo.write(servo_stop);
  Serial.println("======= [End] Transition Push ======="); 
  Serial.println();
}


void runTransitionRetract(int showlog) {
  if (showlog == 1) {
    showActionMessage("Transition Retract...");
  }
  Serial.println("======= [Start] Transition Retract =======");

  myServo.write(servo_backward);
  delay(500); // 500ms เป็นเวลาที่เหมาะสมสำหรับการเคลียร์ตำแหน่งเริ่มต้น

  myServo.write(servo_stop);
  
  // แก้ไขคำให้ถูกต้อง (ของเดิมปรินต์เป็น Push)
  Serial.println("======= [End] Transition Retract ======="); 
  Serial.println();
}
