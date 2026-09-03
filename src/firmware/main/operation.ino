void runStart() {
  showActionMessage("Starting Machine...");
  runDetectPart();       // 1. Wait for ST188 sensor
  runAlignPart();        // 2. Actuator centers part
  runTrigWaitTMX();      // 3. Send to Pi, wait for Pi response
  runSortExecute();
  runTransitionPush();   // 4-6. Actuator retracts, servo pushes, servo retracts
}
