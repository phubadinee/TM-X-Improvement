void runStart() {
  showActionMessage("Starting Machine...");
  runDetectPart();       // 1. Wait for ST188 sensor
  
  extendPart();
  
  char result_ = runTrigWaitTMX_TEST();
  result_ = '0';    
  retractPart();
  runSortExecute(result_);
  runTransitionPush();   // 4-6. Actuator retracts, servo pushes, servo retracts
}

void runSystemHoming() {
  showActionMessage("System Homing...");
  runTransitionPush();
  runHoming();
  for_beep();
}


void runEmergencyHalt() {
  showActionMessage("! EMERGENCY HALT !");
  delay(2000);  // ให้อ่านนานหน่อย
}
