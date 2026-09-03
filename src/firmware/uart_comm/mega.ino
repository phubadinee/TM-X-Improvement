void setup() {
  // Initialize USB serial for communication with Raspberry Pi over the USB cable
  Serial.begin(115200);
}

unsigned long lastSendTime = 0;

void loop() {
  // 1. Check if data is available from Raspberry Pi via USB
  if (Serial.available()) {
    String dataFromPi = Serial.readStringUntil('\n');
    dataFromPi.trim(); // Remove whitespace and newline characters
    
    if (dataFromPi.length() > 0) {
      // Send a response back to the Pi
      Serial.print("Mega processed command: [");
      Serial.print(dataFromPi);
      Serial.println("]");
    }
  }
  
  // 2. Test Mega -> Pi communication 
  // Send a ping to the Pi every 5 seconds
  if (millis() - lastSendTime > 5000) {
    Serial.print("Heartbeat from Mega! Uptime (ms): ");
    Serial.println(millis());
    lastSendTime = millis();
  }
}
