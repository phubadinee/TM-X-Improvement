void runStart() {
  showActionMessage("Starting Machine...");
  runDetectPart();       // 1. Wait for ST188 sensor
  
  extendPart();
  
//  char result_ = runTrigWaitTMX_TEST();
  char result_ = runTrigWaitTMX();
  result_ = '0';    
  retractPart();
  runSortExecute(result_);
  runTransitionPush();   // 4-6. Actuator retracts, servo pushes, servo retracts
}

void runSystemHoming() {
  showActionMessage("System Homing...");

  retractPart();
  
  runTransitionRetract();
  runTransitionPush();
  
  runHoming();
  for_beep();
}


void runEmergencyHalt() {
  // 1. สั่งตัดไฟมอเตอร์ทันทีเพื่อความปลอดภัย
  digitalWrite(enPin, HIGH); 
  
  // 2. สั่งดึงไฟขา 12 ลง GND ซึ่งจะไปดึงขา RESET ของบอร์ดให้ทำงาน
  // บอร์ดจะดับและเปิดใหม่ทันที 100% เหมือนเอานิ้วกดปุ่มรีเซ็ต
  digitalWrite(RESET_PIN, LOW);
}
