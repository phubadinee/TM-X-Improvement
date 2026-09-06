void runTrigWaitTMX() {
  showActionMessage("Measuring (TM-X)...");

  while (Serial.available() > 0) Serial.read();

  // Send to Rasp pi
  Serial.println("<TRIGGER_TMX>");

  bool receivedAck = false;
  while (!receivedAck) {
    if (Serial.available()) {
      String response = Serial.readStringUntil('\n');
      response.trim();

      if (response == "<MEASURE_OK>") {
        for_beep();
      } else {
        long_beep();
      }

      receivedAck = true;

    }
  }

  Serial.println("Measurement finished.");
}
