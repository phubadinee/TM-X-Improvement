int read_st188(){
  int st188_val = analogRead(st188Pin);
  // Serial.print("ST188 Value : ");        
  // Serial.println(st188_val);        
  int st188_val_map = map(st188_val, 0, 1023, 0, 100);
  Serial.print("ST188 Value Map : ");        
  Serial.println(st188_val_map); 

  return st188_val_map;
}

int runDetectPart(){
  showActionMessage("Detecting Part...");
  delay(1000);
  int detect_val = read_st188(); 
  // if (detect_val >= 60){
  //   detect_status = 0;
  //   Serial.println("No Detect");
  // } else {
  //   detect_status = 1;
  //   // for_beep();
  //   Serial.println("Detected");
  // }

  while (detect_val >= 60){
    showActionMessage("No Part...");
    detect_val = read_st188();
    Serial.println(detect_val);
  }
  showActionMessage("Part Detected !!!");
  beep();
  delay(1000);

  // return detect_status;
}

void runAlignPart() {
  showActionMessage("Aligning Part...");
  delay(1000);
}

void runTrigWaitTMX() {
  showActionMessage("Trig & Wait TM-X...");
  delay(1000);
}